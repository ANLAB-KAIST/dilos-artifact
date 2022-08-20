#define _GNU_SOURCE
#include <assert.h>
#include <ddc/prefetch.h>
#include <dict.h>
#include <dlfcn.h>
#include <osv/debug.h>
#include <quicklist.h>
#include <sds.h>
#include <server.h>
#include <stddef.h>
struct RedisCommand {
    enum {
        NONE,
        GET,
        LRANGE,
    } command;
    robj *value;

    union {
        struct {
        } get;
        struct {
            quicklistIter *iter;
            long length;
        } lrange;
    };
};

static __thread struct RedisCommand current;

static void (*getCommand_inner)(struct client *);
static void (*lrangeCommand_inner)(struct client *);
static robj *(*lookupKeyRead_inner)(struct redisDb *, robj *);
static void (*addReplyArrayLen_inner)(struct client *c, long length);
static quicklistIter *(*quicklistGetIteratorAtIdx_inner)(const quicklist *,
                                                         const int,
                                                         const long long);

void getCommand(struct client *c) {
    current.command = GET;
    getCommand_inner(c);
    current.command = NONE;
    current.value = NULL;
}

void lrangeCommand(struct client *c) {
    current.command = LRANGE;
    lrangeCommand_inner(c);
    current.command = NONE;
    current.value = NULL;
    current.lrange.iter = NULL;
    current.lrange.length = 0;
}

robj *lookupKeyRead(struct redisDb *db, robj *key) {
    robj *ret = lookupKeyRead_inner(db, key);
    current.value = current.command != NONE ? ret : NULL;
    return ret;
}

void addReplyArrayLen(struct client *c, long length) {
    if (current.command == LRANGE) {
        current.lrange.length = length;
    }
    addReplyArrayLen_inner(c, length);
}

quicklistIter *quicklistGetIteratorAtIdx(const quicklist *ql,
                                         const int direction,
                                         const long long idx) {
    quicklistIter *iter = quicklistGetIteratorAtIdx_inner(ql, direction, idx);

    if (current.command == LRANGE) current.lrange.iter = iter;

    return iter;
}

static inline uintptr_t align_down(uintptr_t n, uintptr_t alignment) {
    return n & ~(alignment - 1);
}
static inline size_t max(size_t a, size_t b) { return a > b ? a : b; }

static inline int prefetch_range(const struct ddc_event_t *event, char *s,
                                 size_t length, uintptr_t skip) {
    current.value = NULL;
    uintptr_t start = align_down((uintptr_t)s, PREFETCH_PAGE_SIZE);
    uintptr_t end = align_down(((uintptr_t)s + length - 1), PREFETCH_PAGE_SIZE);
    uintptr_t fault_page = align_down(event->fault_addr, PREFETCH_PAGE_SIZE);
    for (; start <= end; start += PREFETCH_PAGE_SIZE) {
        if (start == fault_page || start == skip) continue;

        struct ddc_prefetch_t command;
        command.type = DDC_PREFETCH_PAGE;
        command.addr = start;
        enum ddc_prefetch_result_t ret = ddc_prefetch(event, &command);

        int do_break = 0;
        switch (ret) {
            case DDC_RESULT_ERR_GIVEUP:
            case DDC_RESULT_ERR_NOT_DDC:
            case DDC_RESULT_ERR_UNKNOWN:
                debug_early_u64("prefetch addr: ", (uint64_t)command.addr);
                debug_early_u64("prefetch error: ", (uint64_t)ret);
                abort();
            case DDC_RESULT_ERR_QP_FULL:
                do_break = 1;
                break;
            default:
                break;
        }
        if (do_break) break;
    }

    return 0;
}

/* GET RELATED */

static inline int handler_redis_get(const struct ddc_event_t *event) {
    if (current.command != GET) return 0;
    if (current.value->encoding != OBJ_ENCODING_RAW) return 0;

    // if (align_down((uintptr_t)current.value->ptr, PREFETCH_PAGE_SIZE) !=
    //     align_down(event->fault_addr, PREFETCH_PAGE_SIZE))
    //     return 0;

    char buffer[sizeof(struct sdshdr32) + 1];

    struct ddc_prefetch_t command;
    command.type = DDC_PREFETCH_SUBPAGE;
    command.addr =
        max(((uintptr_t)current.value->ptr) - (sizeof(struct sdshdr32)),
            align_down((uintptr_t)current.value->ptr, PREFETCH_PAGE_SIZE));
    command.sub_page.buff = (void *)buffer;
    command.sub_page.length = (uintptr_t)current.value->ptr - command.addr;
    enum ddc_prefetch_result_t ret = ddc_prefetch(event, &command);

    switch (ret) {
        case DDC_RESULT_OK_LOCAL: {
            sds s = (sds)(buffer + command.sub_page.length);
            if ((s[-1] & SDS_TYPE_MASK) == SDS_TYPE_64) return 0;
            if ((s[-1] & SDS_TYPE_MASK) == SDS_TYPE_32 &&
                command.sub_page.length < sizeof(struct sdshdr32))
                return 0;
            if ((s[-1] & SDS_TYPE_MASK) == SDS_TYPE_16 &&
                command.sub_page.length < sizeof(struct sdshdr16))
                return 0;
            if ((s[-1] & SDS_TYPE_MASK) == SDS_TYPE_8 &&
                command.sub_page.length < sizeof(struct sdshdr8))
                return 0;
            size_t length = sdslen(s);
            uintptr_t skip = align_down(command.addr, PREFETCH_PAGE_SIZE);

            return prefetch_range(event, (char *)current.value->ptr, length,
                                  skip);
        }
        case DDC_RESULT_ERR_UNKNOWN:
            debug_early_u64("prefetch addr: ", (uint64_t)command.addr);
            debug_early_u64("prefetch error: ", (uint64_t)ret);
            abort();
        case DDC_RESULT_ERR_NOT_DDC:
        case DDC_RESULT_ERR_GIVEUP:
        case DDC_RESULT_ERR_QP_FULL:
            return 0;
        default:
            break;
    }

    return 0;
}
static inline int handler_redis_subpage_get(const struct ddc_event_t *event) {
    char *buffer = (char *)event->sub_page.sub_page;
    char *sds_v =
        (char *)(event->sub_page.addr + event->sub_page.size_of_sub_page);

    sds s = (sds)(buffer + event->sub_page.size_of_sub_page);
    if ((s[-1] & SDS_TYPE_MASK) == SDS_TYPE_64) return 0;
    if ((s[-1] & SDS_TYPE_MASK) == SDS_TYPE_32 &&
        event->sub_page.size_of_sub_page < sizeof(struct sdshdr32))
        return 0;
    if ((s[-1] & SDS_TYPE_MASK) == SDS_TYPE_16 &&
        event->sub_page.size_of_sub_page < sizeof(struct sdshdr16))
        return 0;
    if ((s[-1] & SDS_TYPE_MASK) == SDS_TYPE_8 &&
        event->sub_page.size_of_sub_page < sizeof(struct sdshdr8))
        return 0;

    size_t length = sdslen(s);
    uintptr_t skip = align_down(event->sub_page.addr, PREFETCH_PAGE_SIZE);
    return prefetch_range(event, sds_v, length, skip);
}

/* GET RELATED END */
/* LRANGE RELATED  */

static inline int prefetch_qucklist_node(const struct ddc_event_t *event,
                                         quicklistNode *node, int offset) {
    // First, copy node (may from remote )
    quicklistNode node_local;

    struct ddc_prefetch_t command;

    command.type = DDC_PREFETCH_SUBPAGE;
    command.addr = ((uintptr_t)node);
    command.sub_page.buff = (void *)&node_local;
    command.sub_page.length = sizeof(node_local);
    command.metadata = offset;
    enum ddc_prefetch_result_t ret = ddc_prefetch(event, &command);

    switch (ret) {
        case DDC_RESULT_OK_LOCAL: {
            size_t length = node_local.sz;
            offset += node_local.count;
            prefetch_range(event, (char *)node_local.zl, length, 0);
            if (offset < current.lrange.length)
                prefetch_qucklist_node(event, node_local.next, offset);
            return 0;
        }
        case DDC_RESULT_ERR_UNKNOWN:
            debug_early_u64("prefetch addr: ", (uint64_t)command.addr);
            debug_early_u64("prefetch error: ", (uint64_t)ret);
            abort();
        case DDC_RESULT_ERR_NOT_DDC:
        case DDC_RESULT_ERR_GIVEUP:
        case DDC_RESULT_ERR_QP_FULL:
            return 0;
        default:
            break;
    }

    return 0;
}

static inline int handler_redis_subpage_lrange(
    const struct ddc_event_t *event) {
    quicklistNode *node = (quicklistNode *)event->sub_page.sub_page;
    int offset = event->sub_page.metadata;
    size_t length = node->sz;

    offset += node->count;
    prefetch_range(event, (char *)node->zl, length, 0);
    if (offset < current.lrange.length)
        prefetch_qucklist_node(event, node->next, offset);
    return 0;
}

static inline int handler_redis_lrange(const struct ddc_event_t *event) {
    if (current.command != LRANGE) return 0;
    if (current.lrange.iter == NULL) return 0;
    if (current.value->encoding != OBJ_ENCODING_QUICKLIST) return 0;

    // find current offset (below may be safe)
    int offset = 0;
    quicklistNode *node = current.lrange.iter->quicklist->head;
    while (node != current.lrange.iter->current) {
        offset += node->count;
        node = node->next;
    }

    prefetch_qucklist_node(event, node, offset);

    return 0;
}

static inline int handler_redis_subpage(const struct ddc_event_t *event) {
    if (current.value == NULL) return 0;

    switch (current.command) {
        case GET:
            handler_redis_subpage_get(event);
            break;
        case LRANGE:
            handler_redis_subpage_lrange(event);
            break;

        default:
            break;
    }
    return 0;
}

/* LRANGE RELATED END */

static inline int handler_redis_start(const struct ddc_event_t *event) {
    if (current.value == NULL) return 0;
    // if ((char *)current.value->ptr >= mmu::phys_mem) return 0;
    switch (current.command) {
        case GET:
            handler_redis_get(event);
            break;
        case LRANGE:
            handler_redis_lrange(event);
            break;

        default:
            break;
    }
    return 0;
}

static int handler_redis(const struct ddc_event_t *event) {
    switch (event->type) {
        case DDC_EVENT_PREFETCH_START:
            return handler_redis_start(event);
        case DDC_EVENT_SUBPAGE_FETCHED:
            return handler_redis_subpage(event);
        default:
            return 0;
    }
}
void ddc_prefetcher_init() {
    getCommand_inner =
        (void (*)(struct client *))dlsym(RTLD_NEXT, "getCommand");
    if (getCommand_inner == NULL) abort();
    lrangeCommand_inner =
        (void (*)(struct client *))dlsym(RTLD_NEXT, "lrangeCommand");
    if (lrangeCommand_inner == NULL) abort();

    lookupKeyRead_inner = (robj * (*)(struct redisDb *, robj *))
        dlsym(RTLD_NEXT, "lookupKeyRead");
    if (lookupKeyRead_inner == NULL) abort();
    addReplyArrayLen_inner =
        (void (*)(struct client *, long))dlsym(RTLD_NEXT, "addReplyArrayLen");
    if (addReplyArrayLen_inner == NULL) abort();
    quicklistGetIteratorAtIdx_inner =
        (quicklistIter * (*)(const quicklist *, const int, const long long))
            dlsym(RTLD_NEXT, "quicklistGetIteratorAtIdx");
    if (quicklistGetIteratorAtIdx_inner == NULL) abort();

    ddc_register_handler(handler_redis, 0);
}