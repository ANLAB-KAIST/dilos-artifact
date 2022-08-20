#include <infiniband/verbs.h>
#include <netdb.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <algorithm>
#include <boost/circular_buffer.hpp>
#include <cstring>
#include <ddc/debug.hh>
#include <ddc/options.hh>
#include <ddc/phys.hh>
#include <ddc/remote.hh>
#include <map>
#include <memory>
#include <remote/common.hh>
#include <remote/defs.hh>

// #define ENABLE_DELAY
// #define ENABLE_NET_COUNT

#ifndef REMOTE_DELAY_CYCLES
#define REMOTE_DELAY_CYCLES 14000
#endif

#ifdef ENABLE_DELAY

static inline uint64_t get_cycles() {
    uint32_t low, high;
    uint64_t val;
    asm volatile("rdtsc" : "=a"(low), "=d"(high));
    val = high;
    val = (val << 32) | low;
    return val;
}

#endif

// #define DRV_DEBUG

#define PAGE_SIZE (4096)
#define MAX_MR (16)

#define ABORT(...)           \
    do {                     \
        printf(__VA_ARGS__); \
        abort();             \
    } while (0);

namespace ddc {

using namespace remote;

struct mr_meta_t {
    uintptr_t start;
    size_t end;
    uint32_t lkey;
    struct ibv_mr *mr;
};
struct server_t : peer_t {
    uintptr_t start;
    size_t len;
    uint32_t rkey;
    std::array<struct mr_meta_t, MAX_MR> mrs;
};
static ctx_t ctx;

static server_t remote_server;

/* CONTROL PATH */

static bool send_request(request &req, response &resp) {
    int wc = 0;
    unsigned long total_write_bytes = 0;
    ssize_t write_bytes = 0;
    req.convert();
    while (!wc && total_write_bytes < sizeof(req)) {
        write_bytes = write(remote_server.peer_sock, &req,
                            sizeof(req) - total_write_bytes);
        if (write_bytes > 0)
            total_write_bytes += write_bytes;
        else if (write_bytes == 0)
            wc = 1;
        else
            wc = write_bytes;
    }
    req.convert();

    if (wc > 0) return false;

    int rc = 0;
    unsigned long total_read_bytes = 0;
    ssize_t read_bytes = 0;
    memset(&resp, 0, sizeof(resp));
    while (!rc && total_read_bytes < sizeof(resp)) {
        read_bytes = read(remote_server.peer_sock, &resp,
                          sizeof(resp) - total_read_bytes);
        if (read_bytes > 0)
            total_read_bytes += read_bytes;
        else if (read_bytes == 0)
            rc = 1;
        else
            rc = read_bytes;
    }

    if (rc > 0) return false;

    resp.convert();

    return true;
}

static void request_connect() {
    request req;
    response resp;
    memset(&req, 0, sizeof(req));

    req.type = request_type::CONNECT;
    req.connect.lid = ctx.port_attr.lid;
    memcpy(req.connect.gid, ctx.gid.raw, sizeof(ctx.gid.raw));

    bool ret = send_request(req, resp);
    if (!ret) ABORT("fail to send request");

    if (resp.type == response_type::ERR) ABORT("server response error");
    remote_server.lid = resp.connect.lid;
    memcpy(remote_server.gid, resp.connect.gid, sizeof(remote_server.gid));
    remote_server.start = resp.connect.start_addr;
    remote_server.len = resp.connect.len;
    remote_server.rkey = resp.connect.rkey;
}

static bool request_new_qp(uint32_t &qp_num) {
    request req;
    response resp;

    memset(&req, 0, sizeof(req));

    req.type = request_type::NEW_QP;

    if (remote_server.rkey == 0) return false;
    req.new_qp.rkey = remote_server.rkey;

    bool ret = send_request(req, resp);
    if (!ret) return false;

    if (resp.type == response_type::ERR) return false;

    qp_num = resp.new_qp.qp_num;
    return true;
}

bool request_connect_qp(ibv_qp *qp, uint32_t qp_num_remotes) {
    request req;
    response resp;

    memset(&req, 0, sizeof(req));

    req.type = request_type::CONNECT_QP;

    if (remote_server.rkey == 0) return false;
    req.connect_qp.rkey = remote_server.rkey;
    req.connect_qp.qp_num_client = qp->qp_num;
    req.connect_qp.qp_num_remote = qp_num_remotes;

    bool ret = send_request(req, resp);
    if (!ret) return false;

    if (resp.type == response_type::ERR) return false;
    return true;
}

/* CONTROL PATH END */

static struct ibv_qp_ex *create_qp(struct ibv_cq *cq, int max_send_wr,
                                   int max_recv_wr, int max_send_sge,
                                   int max_recv_sge) {
    if (max_send_wr > ctx.device_attr.max_qp_wr) ABORT("too long wr");
    if (max_recv_wr > ctx.device_attr.max_qp_wr) ABORT("too long wr");
    if (max_send_sge > ctx.device_attr.max_sge) ABORT("too long sge");
    if (max_recv_sge > ctx.device_attr.max_sge) ABORT("too long sge");

    bool ret, reti;
    struct ibv_qp_init_attr_ex qp_init_attr_ex;
    memset(&qp_init_attr_ex, 0, sizeof(qp_init_attr_ex));
    qp_init_attr_ex.send_ops_flags |=
        IBV_QP_EX_WITH_RDMA_READ | IBV_QP_EX_WITH_RDMA_WRITE;
    qp_init_attr_ex.pd = remote_server.pd;
    qp_init_attr_ex.comp_mask |=
        IBV_QP_INIT_ATTR_SEND_OPS_FLAGS | IBV_QP_INIT_ATTR_PD;

    qp_init_attr_ex.send_cq = cq;
    qp_init_attr_ex.recv_cq = cq;

    qp_init_attr_ex.cap.max_send_wr = max_send_wr;
    qp_init_attr_ex.cap.max_send_sge = max_send_sge;
    qp_init_attr_ex.cap.max_recv_wr = max_recv_wr;
    qp_init_attr_ex.cap.max_recv_sge = max_recv_sge;
    qp_init_attr_ex.cap.max_inline_data = -1;
    qp_init_attr_ex.qp_type = IBV_QPT_RC;
    qp_init_attr_ex.srq = NULL;
    qp_init_attr_ex.sq_sig_all = 1;  // TODO NON SIG ALL
    struct ibv_qp *qp = ibv_create_qp_ex(ctx.ib_ctx, &qp_init_attr_ex);
    assert(qp != NULL);
    struct ibv_qp_ex *qpx = ibv_qp_to_qp_ex(qp);

    uint32_t remote_qp_num = 0;
    ret = request_new_qp(remote_qp_num);
    assert(ret == true);

    struct ibv_qp_attr attr;

    /* Modify QP to INIT */
    memset(&attr, 0, sizeof(attr));
    attr.qp_state = IBV_QPS_INIT;
    attr.port_num = ctx.port_id;
    attr.pkey_index = cfg::pkey_index;
    attr.qp_access_flags = cfg::client::qp_access_flags;
    reti = ibv_modify_qp(qp, &attr, cfg::client::qp_init_flags);
    assert(reti == 0);
    /* Modify QP to INIT (END) */

    /* Modify QP to RTR */
    memset(&attr, 0, sizeof(attr));
    attr.qp_state = IBV_QPS_RTR;
    attr.path_mtu = cfg::mtu;
    attr.dest_qp_num = remote_qp_num;
    attr.rq_psn = cfg::rq_psn;
    attr.max_dest_rd_atomic = cfg::max_dest_rd_atomic;
    attr.min_rnr_timer = cfg::min_rnr_timer;
    attr.ah_attr.dlid = remote_server.lid;
    attr.ah_attr.sl = cfg::sl;
    attr.ah_attr.src_path_bits = cfg::src_path_bits;
    attr.ah_attr.port_num = ctx.port_id;

    // Always have GID
    attr.ah_attr.is_global = 1;
    memcpy(&attr.ah_attr.grh.dgid, remote_server.gid, 16);
    attr.ah_attr.grh.flow_label = cfg::flow_label;
    attr.ah_attr.grh.hop_limit = cfg::hop_limit;
    attr.ah_attr.grh.sgid_index = ctx.gid_idx;
    attr.ah_attr.grh.traffic_class = cfg::traffic_class;

    reti = ibv_modify_qp(qp, &attr, cfg::client::qp_rtr_flags);
    assert(reti == 0);
    /* Modify QP to RTR (END) */

    /* Modify QP to RTS */
    memset(&attr, 0, sizeof(attr));
    attr.qp_state = IBV_QPS_RTS;
    attr.timeout = cfg::timeout;
    attr.retry_cnt = cfg::retry_cnt;
    attr.rnr_retry = cfg::rnr_retry;
    attr.sq_psn = cfg::sq_psn;
    attr.max_rd_atomic = cfg::max_rd_atomic;
    reti = ibv_modify_qp(qp, &attr, cfg::client::qp_rts_flags);
    assert(reti == 0);
    /* Modify QP to RTS (END) */

    ret = request_connect_qp(qp, remote_qp_num);
    assert(ret == true);

    return qpx;
}

void remote_queue::setup() {
    int cq_size = 0;

    for (auto &qp : fetch_qp) {
        cq_size += qp.max;
    }
    for (auto &qp : push_qp) {
        cq_size += qp.max;
    }

    if (cq_size < 1) ABORT("zero-sized cq");
    if (cq_size > ctx.device_attr.max_cqe) ABORT("cq too long");
    struct ibv_cq *ib_cq = ibv_create_cq(ctx.ib_ctx, cq_size, NULL, NULL, 0);
    cq = reinterpret_cast<void *>(ib_cq);

#ifdef ENABLE_DELAY
    complete.set_capacity(cq_size);
#endif

#ifdef ENABLE_NET_COUNT
    fetch_total = 0;
    push_total = 0;
#endif

    // create and connect qps

    for (auto &qp : fetch_qp) {
        struct ibv_qp_ex *ib_qp =
            create_qp(ib_cq, qp.max, 0, cfg::client::max_sge, 10);
        qp.inner = reinterpret_cast<void *>(ib_qp);
    }
    for (auto &qp : push_qp) {
        struct ibv_qp_ex *ib_qp =
            create_qp(ib_cq, qp.max, 0, cfg::client::max_sge, 10);
        qp.inner = reinterpret_cast<void *>(ib_qp);
    }
}
remote_queue::~remote_queue() {
    for (auto &qp : fetch_qp) {
        ibv_destroy_qp(reinterpret_cast<struct ibv_qp *>(qp.inner));
    }
    for (auto &qp : push_qp) {
        ibv_destroy_qp(reinterpret_cast<struct ibv_qp *>(qp.inner));
    }

    ibv_destroy_cq(reinterpret_cast<struct ibv_cq *>(cq));
}

inline static struct mr_meta_t &find_mr(void *paddr, size_t size) {
    auto it = remote_server.mrs.begin();
    while (it->start != 0) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(paddr);
        if (addr >= it->start && addr < it->end) {
            return *it;
        }
        ++it;
    }
    printf("paddr: %p\n ", paddr);
    abort();
}

bool remote_queue::fetch(int qp_id, uintptr_t token, void *paddr,
                         uintptr_t offset, size_t size) {
    auto &qp = fetch_qp[qp_id];
    struct ibv_qp_ex *qpx = reinterpret_cast<ibv_qp_ex *>(qp.inner);
#ifdef DRV_DEBUG
    ddc_debug("fetch: ", token);
    // printf("[%d: %lx] %p <- %lx (%lx)\n", qp_id, token, paddr, offset, size);
#endif
    qpx->wr_id = token;
    qpx->wr_flags = IBV_SEND_SIGNALED;

    auto &meta = find_mr(paddr, size);
    ibv_wr_start(qpx);
    ibv_wr_rdma_read(qpx, remote_server.rkey, remote_server.start + offset);
    ibv_wr_set_sge(qpx, meta.lkey, (uint64_t)paddr, size);
    int ret = ibv_wr_complete(qpx);
    assert(ret == 0);

    // #ifdef ENABLE_DELAY
    //     qp.cycles.push_back({token, get_cycles() + REMOTE_DELAY_CYCLES});
    // #endif

#ifdef ENABLE_NET_COUNT
    fetch_total += size;
#endif

    return true;
}

bool remote_queue::push(int qp_id, uintptr_t token, void *paddr,
                        uintptr_t offset, size_t size) {
    auto &qp = push_qp[qp_id];
    struct ibv_qp_ex *qpx = reinterpret_cast<ibv_qp_ex *>(qp.inner);
#ifdef DRV_DEBUG

    ddc_debug("push: ", token);
    // printf("[%d: %lx] %p -> %lx or %lx (%lx) : %lx\n", qp_id, token, paddr,
    //        offset, remote_server.start + offset, size, *(uintptr_t
    //        *)(paddr));
#endif
    qpx->wr_id = token;
    qpx->wr_flags = IBV_SEND_SIGNALED;

    auto &meta = find_mr(paddr, size);
    ibv_wr_start(qpx);
    ibv_wr_rdma_write(qpx, remote_server.rkey, remote_server.start + offset);
    ibv_wr_set_sge(qpx, meta.lkey, (uint64_t)paddr, size);
    int ret = ibv_wr_complete(qpx);
    assert(ret == 0);

    // #ifdef ENABLE_DELAY
    //     qp.cycles.push_back({token, get_cycles() + REMOTE_DELAY_CYCLES});
    // #endif

#ifdef ENABLE_NET_COUNT
    push_total += size;
#endif

    return true;
}
int remote_queue::poll(uintptr_t tokens[], int len) {
    struct ibv_wc wc[len];
    int polled = ibv_poll_cq(reinterpret_cast<struct ibv_cq *>(cq), len, wc);
    int i = 0;
    while (i < polled && i < len) {
#ifdef DRV_DEBUG
        ddc_remote_log_poll(wc[i].wr_id);
#endif
#ifndef ENABLE_DELAY
        tokens[i] = wc[i].wr_id;
#else
        complete.push_back({wc[i].wr_id, get_cycles() + REMOTE_DELAY_CYCLES});
#endif
        if (wc[i].status != IBV_WC_SUCCESS) {
            ddc_debug("status: ", wc[i].status);
            ddc_debug("token: ", tokens[i]);
            abort();
        }
        assert(wc[i].status == IBV_WC_SUCCESS);
        ++i;
    }

#ifdef ENABLE_DELAY
    i = 0;
    while (i < len && !complete.empty()) {
        auto &f = complete.front();
        auto current = get_cycles();
        if (current < f.cycle) {
            break;
        }
        tokens[i] = f.token;
        ++i;
        complete.pop_front();
    }
#endif
    return i;
}

static inline size_t setup_sge(struct ibv_qp_ex *qpx, uint32_t lkey,
                               uint64_t paddr, uint64_t vec) {
    struct ibv_sge sg_list[cfg::client::max_sge];
    uint64_t vec2 = vec;
    uint64_t count = vec & 0x7F;
    vec >>= 7;
    uint64_t start = vec & 0x7F;
    vec >>= 7;
    int num_sge = 0;

    size_t size = 0;

    while (count != 0) {
        sg_list[num_sge].addr = paddr + (start << 6);
        sg_list[num_sge].length = count << 6;
        sg_list[num_sge].lkey = lkey;
#ifdef DRV_DEBUG
        ddc_remote_log_sge(sg_list[num_sge].addr, sg_list[num_sge].length,
                           sg_list[num_sge].lkey);
#endif
        size += sg_list[num_sge].length;
        ++num_sge;

        count = vec & 0x7F;
        vec >>= 7;
        start = vec & 0x7F;
        vec >>= 7;
    }
    assert(num_sge != 0);

    ibv_wr_set_sge_list(qpx, num_sge, sg_list);
    return size;
}

bool remote_queue::fetch_vec(int qp_id, uintptr_t token, void *paddr,
                             uintptr_t offset, uintptr_t vec) {
    auto &qp = fetch_qp[qp_id];
    struct ibv_qp_ex *qpx = reinterpret_cast<ibv_qp_ex *>(qp.inner);
#ifdef DRV_DEBUG
    ddc_remote_log_fetch(paddr, token, offset, vec);
    ddc_debug("fetch vec: ", token);
    // printf("[%d: %lx] %p <- %lx (%lx)\n", qp_id, token, paddr, offset, vec);
#endif
    qpx->wr_id = token;
    qpx->wr_flags = IBV_SEND_SIGNALED;
    assert((uintptr_t)paddr % PAGE_SIZE == 0);

    auto &meta = find_mr(paddr, 0);
    ibv_wr_start(qpx);
    ibv_wr_rdma_read(qpx, remote_server.rkey, remote_server.start + offset);

#ifndef ENABLE_NET_COUNT
    setup_sge(qpx, meta.lkey, (uint64_t)paddr, vec);
#else
    fetch_total += setup_sge(qpx, meta.lkey, (uint64_t)paddr, vec);
#endif
    int ret = ibv_wr_complete(qpx);
    assert(ret == 0);

    // #ifdef ENABLE_DELAY
    //     qp.cycles.push_back({token, get_cycles() + REMOTE_DELAY_CYCLES});
    // #endif

    return true;
}

bool remote_queue::push_vec(int qp_id, uintptr_t token, void *paddr,
                            uintptr_t offset, uintptr_t vec) {
    auto &qp = push_qp[qp_id];
    struct ibv_qp_ex *qpx = reinterpret_cast<ibv_qp_ex *>(qp.inner);
#ifdef DRV_DEBUG
    ddc_remote_log_push(paddr, token, offset, vec);
    // printf("[%d: %lx] %p -> %lx or %lx (%lx) : %lx\n", qp_id, token, paddr,
    //        offset, remote_server.start + offset, vec, *(uintptr_t *)(paddr));
#endif
    qpx->wr_id = token;
    qpx->wr_flags = IBV_SEND_SIGNALED;

    auto &meta = find_mr(paddr, 0);
    ibv_wr_start(qpx);
    ibv_wr_rdma_write(qpx, remote_server.rkey, remote_server.start + offset);
#ifndef ENABLE_NET_COUNT
    setup_sge(qpx, meta.lkey, (uint64_t)paddr, vec);
#else
    push_total += setup_sge(qpx, meta.lkey, (uint64_t)paddr, vec);
#endif
    int ret = ibv_wr_complete(qpx);
    assert(ret == 0);

    // #ifdef ENABLE_DELAY
    //     qp.cycles.push_back({token, get_cycles() + REMOTE_DELAY_CYCLES});
    // #endif

    return true;
}

void socket_init() {
    struct addrinfo *resolved_addr = NULL;
    struct addrinfo *iterator;
    char service[32];

    struct addrinfo hints = {.ai_flags = AI_PASSIVE,
                             .ai_family = AF_INET,
                             .ai_socktype = SOCK_STREAM};

    if (sprintf(service, "%d", options::ms_port) < 0) {
        ABORT("snprintf fail");
    }

    int ret =
        getaddrinfo(options::ms_ip.c_str(), service, &hints, &resolved_addr);
    if (ret < 0) {
        ABORT("%s for %s:%d\n", gai_strerror(ret), options::ms_ip.c_str(),
              options::ms_port);
    }
    remote_server.peer_sock = -1;
    for (iterator = resolved_addr; iterator; iterator = iterator->ai_next) {
        remote_server.peer_sock = socket(
            iterator->ai_family, iterator->ai_socktype, iterator->ai_protocol);
        if (remote_server.peer_sock >= 0) {
            if ((ret = connect(remote_server.peer_sock, iterator->ai_addr,
                               iterator->ai_addrlen) == -1)) {
                fprintf(stdout, "failed connect :%s\n", strerror(errno));
                close(remote_server.peer_sock);
                remote_server.peer_sock = -1;
                continue;
            }
            break;
        }
    }
    if (resolved_addr) freeaddrinfo(resolved_addr);
    assert(remote_server.peer_sock > 0);
}

void remote_init() {
    // connect MS
    init_ib(ctx, options::ib_device, options::ib_port, options::gid_idx);

    remote_server.pd = ibv_alloc_pd(ctx.ib_ctx);
    if (remote_server.pd == NULL) {
        ABORT("fail to alloc pd");
    }

    size_t phy_len = 0;
    auto &ranges = memory::get_phys_list(phy_len);

    // register local mrs

    auto it = remote_server.mrs.begin();
    while (it->start != 0) {
        ++it;
    }
    for (size_t i = 0; i < phy_len; ++i) {
        auto &range = ranges[i];
        ibv_mr *mr = ibv_reg_mr(remote_server.pd, range.first, range.second,
                                cfg::client::mr_flags);

        uintptr_t addr = reinterpret_cast<uintptr_t>(range.first);

        it->start = addr;
        it->end = addr + range.second;
        it->lkey = mr->lkey;
        it->mr = mr;
        ++it;
    }
    std::sort(remote_server.mrs.begin(), it,
              [](const struct mr_meta_t &a, const struct mr_meta_t &b) {
                  return a.start < b.start;
              });

    // socket init
    socket_init();

    // connect to server
    request_connect();
}

size_t remote_reserve_size() { return 0; }

}  // namespace ddc
