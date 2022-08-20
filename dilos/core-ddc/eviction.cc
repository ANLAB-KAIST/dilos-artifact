#include <ddc/eviction.h>

#include <ddc/options.hh>
#include <ddc/remote.hh>
#include <osv/mempool.hh>

#include "cleaner.hh"
#include "init.hh"
#include "page.hh"
#include "pte.hh"

// #define NO_SG

TRACEPOINT(trace_ddc_evict_request_memory, "size=%lx, hard=%d", size_t, bool);
TRACEPOINT(trace_ddc_evict_request_memory_done, "size=%lx, hard=%d", size_t,
           bool);
TRACEPOINT(trace_ddc_evict_get_mask, "mask=%lx", uintptr_t);
TRACEPOINT(trace_ddc_evict_get_mask_retrun, "mask=%lx", uintptr_t);
TRACEPOINT(trace_ddc_evict_max_zeros, "zeros=%d %d", int, int);
TRACEPOINT(trace_ddc_evict_max_starts, "starts=%d %d", int, int);
TRACEPOINT(trace_ddc_evict_mask, "mask=%lx", uintptr_t);
TRACEPOINT(trace_ddc_evict_vec, "va=%lx vec=%lx", uintptr_t, uintptr_t);
extern "C" {

uint64_t ddc_eviction_mask_default(uintptr_t vaddr) {
    return 0xFFFFFFFFFFFFFFFF;
}

static ddc_eviction_mask_f mask_f = ddc_eviction_mask_default;

void ddc_register_eviction_mask(ddc_eviction_mask_f f) { mask_f = f; }

static uint64_t ddc_eviction_get_mask(uintptr_t vaddr) {
    trace_ddc_evict_get_mask(vaddr);

    if (vaddr < ddc::vma_middle) return ddc::remote_vec_full;
    uint64_t mask = mask_f(vaddr);
    if (__builtin_popcountll(mask) > 32) return ddc::remote_vec_full;
    trace_ddc_evict_get_mask_retrun(mask);

    // find two big hole
    mask = ~mask;
    int remain = 64;

    int max_zeros[2] = {0, 0};
    int max_starts[2] = {64, 64};

    int count;
    while (remain > 0) {
        count = __builtin_ctzll(mask);
        if (count > remain) count = remain;

        remain -= count;
        mask = ~mask;
        mask >>= count;

        count = __builtin_ctzll(mask);
        if (count > remain) count = remain;

        // count is zero length
        if (max_zeros[0] < count) {
            max_zeros[1] = max_zeros[0];
            max_starts[1] = max_starts[0];

            max_zeros[0] = count;
            max_starts[0] = 64 - remain;
        } else if (max_zeros[1] < count) {
            max_zeros[1] = count;
            max_starts[1] = 64 - remain;
        }

        remain -= count;
        mask = ~mask;
        mask >>= count;
    }
    trace_ddc_evict_max_zeros(max_zeros[0], max_zeros[1]);
    trace_ddc_evict_max_starts(max_starts[0], max_starts[1]);

    if (max_starts[0] > max_starts[1]) {
        uint64_t tmp = max_starts[0];
        max_starts[0] = max_starts[1];
        max_starts[1] = tmp;

        tmp = max_zeros[0];
        max_zeros[0] = max_zeros[1];
        max_zeros[1] = tmp;
    }

    uint8_t starts[3];
    uint8_t counts[3];
    size_t len = 0;

    if (max_starts[0]) {
        starts[len] = 0;
        counts[len] = max_starts[0];
        ++len;
    }
    if (max_starts[0] + max_zeros[0] < 64) {
        starts[len] = max_starts[0] + max_zeros[0];
        counts[len] = max_starts[1] - starts[len];
        ++len;
    }
    if (max_starts[1] + max_zeros[1] < 64) {
        starts[len] = max_starts[1] + max_zeros[1];
        counts[len] = 64 - starts[len];
        ++len;
    }

    uint64_t vec = ddc::pack_vec(starts, counts, len);

    if (!vec) vec = 1;  // todo

    trace_ddc_evict_mask(vec);
    return vec;
}

uint64_t ddc_eviction_mask_test(uintptr_t vaddr) { return mask_f(vaddr); }
}

namespace ddc {

static remote_queue evict_queue({}, {max_evict});
TRACEPOINT(trace_ddc_eviction_evict, "va=%p bytes=%lx", uintptr_t, uint64_t);

constexpr size_t slice_size = 32;
class evictor : public memory::shrinker, public cleaner {
   public:
    evictor()
        : memory::shrinker("ddc shirinker"),
          cleaner(evict_queue, 0, max_evict) {}
    size_t request_memory(size_t s, bool hard) override;
    void tlb_flush_after(uintptr_t token) override {
        auto &page = ipt.lookup(static_cast<mmu::phys>(token));
        active_buffer.push_back(page);
    }
    void push_after(uintptr_t token) override {
        auto &page = ipt.lookup(static_cast<mmu::phys>(token));
        trace_ddc_eviction_evict(page.va,
                                 *(uint64_t *)(mmu::phys_to_virt(token)));
        clean_buffer.push_back(page);
    }
    state clean_handler(uintptr_t token, mmu::hw_ptep<base_page_level> ptep,
                        mmu::pt_element<base_page_level> old,
                        uintptr_t offset) override {
        // caller will insert to clean
        return state::OK_FINISH;
    }

    state clean_handler(uintptr_t token, mmu::hw_ptep<base_page_level> ptep,
                        mmu::pt_element<base_page_level> old, uintptr_t offset,
                        uintptr_t va, uintptr_t &vec) override {
        // caller will insert to clean
        uintptr_t vec_new = get_vec(va);
        if (vec == vec_new) {
            return state::OK_FINISH;
        }
        vec = vec_new;
        return dirty_handler_inner(token, old, offset, vec);
    }

    uint64_t get_vec(uintptr_t va) override {
        return ddc_eviction_get_mask(va);
    }
    size_t evict_slice();
    size_t clean_slice(bool do_finalize);
    void finalize() {
        while (!tlb_empty() || !cleaner_empty()) {
            tlb_flush();
            poll_all();
        }
    }

   private:
    base_page_slice_t active_buffer;
    base_page_slice_t clean_buffer;
};

size_t evictor::clean_slice(bool do_finalize) {
    base_page_slice_t slice_active;

    page_list.slice_pages_active(slice_active, slice_size);
    if (slice_active.size() == 0) {
        return 0;
    }

    auto page = slice_active.begin();
    base_page_slice_t::iterator next;
    while (page != slice_active.end()) {
        mmu::phys paddr = ipt.lookup(*page);

#ifdef NO_SG
        auto ret = process(paddr, page->ptep, va_to_offset(page->va));
#else
        uint64_t vec = 0;
        auto ret = process_mask(paddr, page->ptep, va_to_offset(page->va),
                                page->va, vec);
#endif
        assert(slice_active.size() < 600);

        switch (ret) {
            case state::OK_SKIP:
                // 다음으로, DONT_NEED는 insert시 삭제
                page++;
                break;
            case state::OK_FINISH:
                // 이미 clean임
#ifndef NO_SG
                page->vec = vec;
#endif
                next = slice_active.erase(page);  // push to clean
                clean_buffer.push_back(*page);
                page = next;
                break;
            case state::OK_TLB_PUSHED:
                // active로 // flush 후에 집어 넣음
                page = slice_active.erase(page);  // do not push back
                break;
            case state::OK_PUSHED:
                // clean으로 // push 후에 집어 넣음
#ifndef NO_SG
                page->vec = vec;
#endif
                page = slice_active.erase(page);  // do not push back
                break;
            case state::PTE_UPDATE_FAIL:
                // 재시도
                continue;
            default:
                abort("noreach");
        }
    }
    if (do_finalize) finalize();
    size_t cleaned = clean_buffer.size() << base_page_shift;

    slice_active.splice(slice_active.end(), active_buffer);
    page_list.push_pages_both(slice_active, clean_buffer);
    assert(slice_active.size() < 600);
    assert(active_buffer.size() < 600);
    return cleaned;
}
size_t evictor::evict_slice() {
    size_t freed = 0;
    base_page_slice_t slice_active;
    base_page_slice_t slice_clean;
    page_list.slice_pages_clean(slice_clean, slice_size);
    if (slice_clean.size() == 0) {
        clean_slice(false);
        page_list.slice_pages_clean(slice_clean, slice_size);
    }

    auto page = slice_clean.begin();
    while (page != slice_clean.end()) {
        auto pte = page->ptep.read();
        if (pte.empty()) {
            // removed by MADV_DONTNEED
            // Skip.
            // this page will be removed by page_list.push_pages
            page++;
            continue;
        }
        assert(pte.valid());

        if (pte.accessed()) {
            auto next = slice_clean.erase(page);
            slice_active.push_back(*page);
            page = next;
            // trace_ddc_eviction_go_accessed(pte.addr());
        } else {
            assert(!pte.dirty());
            assert(page->va);
#ifndef NO_SG
            assert(page->vec);
            trace_ddc_evict_vec(page->va, page->vec);
#endif
            auto pte_new = remote_pte_vec(page->ptep, page->vec);

            if (!page->ptep.compare_exchange(pte, pte_new)) {
                // retry
                continue;
            }
            // trace_ddc_eviction_evicted(pte.addr());
            auto next = slice_clean.erase(page);
            freed += base_page_size;
            page->st = page_status::UNTRACK;
            page_list.free_not_used_page(*page);
            page = next;
        }
    }

    page_list.push_pages_both(slice_active, slice_clean);

    return freed;
}
size_t evictor::request_memory(size_t s, bool hard) {
    // Note: shrinker는 하나의 mutex로 보호가 됨 -> 동기화 걱정이 없다
    // (적어도 eviction remote 리소스는)

    // TODO: make soft eviction
    // TODO:
    //  SOFT: clean only
    //  HARD: clean and evict
    trace_ddc_evict_request_memory(s, hard);
    size_t freed = 0;
    while (freed < s) {
        freed += evict_slice();
    }
    finalize();
    page_list.push_pages_both(active_buffer, clean_buffer);
    if (hard) assert(freed != 0);
    assert(active_buffer.empty() && clean_buffer.empty());
    trace_ddc_evict_request_memory_done(freed, hard);
    return freed;
}

static evictor *_evictor;

void evictor_init() {
    if (options::no_eviction) return;
    evict_queue.setup();

    _evictor = new evictor;
}

void eviction_get_stat(size_t &fetch_total, size_t &push_total) {
    evict_queue.get_stat(fetch_total, push_total);
}

}  // namespace ddc
