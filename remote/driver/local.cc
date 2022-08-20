#include <sys/mman.h>

#include <boost/circular_buffer.hpp>
#include <cstring>
#include <ddc/remote.hh>
#include <memory>

#ifndef REMOTE_LOCAL_SIZE_GB
#define REMOTE_LOCAL_SIZE_GB 16
#warning "Using default local size GB"
#endif

#define REMOTE_LOCAL_SIZE ((1ULL * REMOTE_LOCAL_SIZE_GB) << 30)

// #define DRV_DEBUG

#define PAGE_SIZE (4096)

namespace ddc {

static char *remote_memory = nullptr;

enum class req_type {
    PUSH,
    FETCH,
    PUSH_VEC,
    FETCH_VEC,
};
struct req {
    uintptr_t token;
    void *paddr;
    uintptr_t offset;

    size_t size_or_mask;

    int qp_id;
    req_type type;
};

using cq_t = boost::circular_buffer<req>;

void remote_queue::setup() {
    int cq_size = 0;

    for (auto &qp : fetch_qp) {
        cq_size += qp.max;
    }
    for (auto &qp : push_qp) {
        cq_size += qp.max;
    }

    cq = reinterpret_cast<void *>(new cq_t(cq_size));
}

remote_queue::~remote_queue() { delete reinterpret_cast<cq_t *>(cq); }

bool remote_queue::fetch(int qp_id, uintptr_t token, void *paddr,
                         uintptr_t offset, size_t size) {
    auto &qp = fetch_qp[qp_id];
    if (qp.current >= qp.max) {
        return false;
    }
    assert(offset < REMOTE_LOCAL_SIZE);
#ifdef DRV_DEBUG
    printf("[%d: %lx] %p <- %lx (%lx) : %lx\n", qp_id, token, paddr, offset,
           size, *(uintptr_t *)(remote_memory + offset));
#endif
    auto cq_ptr = reinterpret_cast<cq_t *>(cq);
    cq_ptr->push_back({token, paddr, offset, size, qp_id, req_type::FETCH});
    ++qp.current;
    return true;
}

bool remote_queue::push(int qp_id, uintptr_t token, void *paddr,
                        uintptr_t offset, size_t size) {
    auto &qp = push_qp[qp_id];
    if (qp.current >= qp.max) {
        return false;
    }
    assert(offset < REMOTE_LOCAL_SIZE);
#ifdef DRV_DEBUG
    printf("[%d: %lx] %p -> %lx (%lx) : %lx\n", qp_id, token, paddr, offset,
           size, *(uintptr_t *)(paddr));
#endif
    auto cq_ptr = reinterpret_cast<cq_t *>(cq);
    cq_ptr->push_back({token, paddr, offset, size, qp_id, req_type::PUSH});
    ++qp.current;
    return true;
}
bool remote_queue::fetch_vec(int qp_id, uintptr_t token, void *paddr,
                             uintptr_t offset, uintptr_t vec) {
    auto &qp = fetch_qp[qp_id];
    if (qp.current >= qp.max) {
        return false;
    }
    assert(offset < REMOTE_LOCAL_SIZE);
    assert(reinterpret_cast<uintptr_t>(paddr) % PAGE_SIZE == 0);
    assert(offset % PAGE_SIZE == 0);

    auto cq_ptr = reinterpret_cast<cq_t *>(cq);
    cq_ptr->push_back({token, paddr, offset, vec, qp_id, req_type::FETCH_VEC});
    ++qp.current;
    return true;
}
bool remote_queue::push_vec(int qp_id, uintptr_t token, void *paddr,
                            uintptr_t offset, uintptr_t vec) {
    auto &qp = push_qp[qp_id];
    if (qp.current >= qp.max) {
        return false;
    }
    assert(offset < REMOTE_LOCAL_SIZE);
    assert(reinterpret_cast<uintptr_t>(paddr) % PAGE_SIZE == 0);
    assert(offset % PAGE_SIZE == 0);

    auto cq_ptr = reinterpret_cast<cq_t *>(cq);
    cq_ptr->push_back({token, paddr, offset, vec, qp_id, req_type::PUSH_VEC});
    ++qp.current;
    return true;
}

void copy_with_vec(void *dest, void *src, size_t vec) {
    // mask |= 0x3;
    // // ignore first two bits
    // mask = ~mask;
    // uintptr_t start = 0;
    // size_t count = 0;

    // while (start < 64) {
    //     count = __builtin_ctzll(mask);
    //     memcpy((uint8_t *)dest + (start << 6), (uint8_t *)src + (start << 6),
    //            count << 6);
    //     mask = ~mask;
    //     mask >>= count;
    //     start += count;
    //     count = __builtin_ctzll(mask);
    //     mask >>= count;
    //     start += count;
    //     mask = ~mask;
    // }

    uint64_t count = vec & 0x7F;
    vec >>= 7;
    uint64_t start = vec & 0x7F;
    vec >>= 7;
    int num_sge = 0;

    while (count != 0) {
        memcpy((uint8_t *)dest + (start << 6), (uint8_t *)src + (start << 6),
               count << 6);
        ++num_sge;

        count = vec & 0x7F;
        vec >>= 7;
        start = vec & 0x7F;
        vec >>= 7;
    }
    assert(num_sge != 0);
}

int remote_queue::poll(uintptr_t tokens[], int len) {
    int poped = 0;
    auto cq_ptr = reinterpret_cast<cq_t *>(cq);
    while (len > poped && !cq_ptr->empty()) {
        auto &req = cq_ptr->front();
        switch (req.type) {
            case req_type::PUSH:
                memcpy(remote_memory + req.offset, req.paddr, req.size_or_mask);
                push_qp[req.qp_id].current -= 1;
                break;
            case req_type::FETCH:
                memcpy(req.paddr, remote_memory + req.offset, req.size_or_mask);
                fetch_qp[req.qp_id].current -= 1;
                break;
            case req_type::PUSH_VEC:
                copy_with_vec(remote_memory + req.offset, req.paddr,
                              req.size_or_mask);
                push_qp[req.qp_id].current -= 1;
                break;
            case req_type::FETCH_VEC:
                copy_with_vec(req.paddr, remote_memory + req.offset,
                              req.size_or_mask);
                fetch_qp[req.qp_id].current -= 1;
                break;
            default:
                abort();
        }
        tokens[poped] = req.token;
        cq_ptr->pop_front();
        poped += 1;
    }
    return poped;
}

void remote_init() {
    remote_memory = (char *)mmap(
        (void *)0x600000000000ull, REMOTE_LOCAL_SIZE, PROT_READ | PROT_WRITE,
        MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
}

size_t remote_reserve_size() { return REMOTE_LOCAL_SIZE; }

}  // namespace ddc
