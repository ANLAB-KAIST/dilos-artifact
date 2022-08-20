#include "../core/mmu.cc"
// MMU

#include <ddc/mman.h>
#include <ddc/prefetch.h>
#include <dlfcn.h>
#include <osv/mutex.h>

#include <cstddef>
#include <ddc/elf.hh>
#include <ddc/memory.hh>
#include <ddc/mmu.hh>
#include <ddc/remote.hh>
#include <ddc/stat.hh>
#include <osv/migration-lock.hh>
#include <osv/preempt-lock.hh>
#include <osv/sched.hh>

#include "cleaner.hh"
#include "init.hh"
#include "page.hh"
#include "prefetcher/general.hh"
#include "pte.hh"
#include "tlb.hh"
// DDC PT Operations

#include <x86intrin.h>

// #define NORETURN_UNTIL_PREFETCH

// #define NO_SG

STAT_COUNTER(ddc_page_fault);
STAT_COUNTER(ddc_page_fault_remote);
STAT_COUNTER(ddc_page_fault_protected_other);
STAT_AVG(ddc_page_fault_protected_num);
STAT_ELAPSED(ddc_page_fault_remote);
STAT_ELAPSED(ddc_page_fault_remote_real);
STAT_ELAPSED(ddc_page_prefetching);

namespace ddc {

constexpr unsigned ddc_perm = mmu::perm_rw;
constexpr size_t prefetch_window = max_req_page;

TRACEPOINT(trace_ddc_mmu_mmap, "addr=%p, length=%lx, prot=%d, flags=%d", void *,
           size_t, int, int);
TRACEPOINT(trace_ddc_mmu_madvise, "addr=%p, length=%lx, advise=%d", void *,
           size_t, int);
TRACEPOINT(trace_ddc_mmu_mprotect, "addr=%p, length=%lx, prot=%d", void *,
           size_t, int);
TRACEPOINT(trace_ddc_mmu_munmap, "addr=%p, length=%lx", void *, size_t);
TRACEPOINT(trace_ddc_mmu_msync, "addr=%p, length=%lx, flags=%d", void *, size_t,
           int);
TRACEPOINT(trace_ddc_mmu_mincore, "addr=%p, length=%lx", void *, size_t);
TRACEPOINT(trace_ddc_mmu_mlock, "addr=%p, length=%lx", const void *, size_t);
TRACEPOINT(trace_ddc_mmu_munlock, "addr=%p, length=%lx", const void *, size_t);
TRACEPOINT(trace_ddc_mmu_fault, "addr=%p", uintptr_t);
TRACEPOINT(trace_ddc_mmu_fault_first, "va=%p", uintptr_t);
TRACEPOINT(trace_ddc_mmu_fault_remote, "va=%p", uintptr_t);
TRACEPOINT(trace_ddc_mmu_fault_protected, "protector=%d", unsigned);
TRACEPOINT(trace_ddc_mmu_fault_remote_offset, "va=%p", uintptr_t);
TRACEPOINT(trace_ddc_mmu_fault_remote_init_done, "va=%p bytes=%lx", uintptr_t,
           uint64_t);
TRACEPOINT(trace_ddc_mmu_fault_remote_page_done, "va=%p bytes=%lx", uintptr_t,
           uint64_t);
TRACEPOINT(trace_ddc_mmu_polled_size, "size: %ld", size_t);
TRACEPOINT(trace_ddc_mmu_polled, "tag: %d, offset: %lx", int, uintptr_t);
TRACEPOINT(trace_ddc_mmu_polled_page, "offset: %lx size: %ld", uintptr_t,
           size_t);
TRACEPOINT(trace_ddc_mmu_polled_page_poped, "offset: %lx", uintptr_t);
TRACEPOINT(trace_ddc_mmu_prefetch_do_prefetch, "addr=%p", uintptr_t);
TRACEPOINT(trace_ddc_mmu_prefetch_first, "va=%p", uintptr_t);
TRACEPOINT(trace_ddc_mmu_prefetch_remote, "va=%p", uintptr_t);
TRACEPOINT(trace_ddc_mmu_prefetch_remote_offset, "va=%p paddr=%p", uintptr_t,
           void *);
TRACEPOINT(trace_ddc_mmu_prefetch_remote_offset_page, "va=%p paddr=%p",
           uintptr_t, void *);
TRACEPOINT(trace_ddc_mmu_prefetch_remote_page, "va=%p", uintptr_t);
TRACEPOINT(trace_ddc_mmu_prefetch_remote_sub_page, "va=%p", uintptr_t);
TRACEPOINT(trace_ddc_mmu_prefetch_remote_result, "result=%d", int);

class mock_vma : public vma {
   public:
    mock_vma(addr_range range) : vma(range, 0, 0, false, nullptr) {}
    virtual void fault(uintptr_t addr, exception_frame *ef) override {
        abort();
    }
    virtual void split(uintptr_t edge) override { abort(); }
    virtual error sync(uintptr_t start, uintptr_t end) override { abort(); }
    virtual page_allocator *page_ops() override { abort(); }
};

struct request_base {
    void *paddr;
    uintptr_t va;
    int &to_decrease;
    request_base(void *paddr, uintptr_t va, int &to_decrease)
        : paddr(paddr), va(va), to_decrease(to_decrease) {}

    request_base &operator=(const request_base &other) {
        paddr = other.paddr;
        va = other.va;
        to_decrease = other.to_decrease;
        return *this;
    }
};

struct request_init : request_base {
    mmu::hw_ptep<base_page_level> ptep;
    request_init(void *paddr, uintptr_t va, int &to_decrease,
                 mmu::hw_ptep<base_page_level> ptep)
        : request_base(paddr, va, to_decrease), ptep(ptep) {}
    request_init &operator=(const request_init &other) {
        paddr = other.paddr;
        va = other.va;
        to_decrease = other.to_decrease;
        ptep = other.ptep;
        return *this;
    }
};

struct request_page : request_base {
    mmu::hw_ptep<base_page_level> ptep;
    request_page(void *paddr, uintptr_t va, int &to_decrease,
                 mmu::hw_ptep<base_page_level> ptep)
        : request_base(paddr, va, to_decrease), ptep(ptep) {}
};

struct request_subpage : request_base {
    size_t size;
    uintptr_t fault_addr;
    unsigned long metadata;
    request_subpage(void *paddr, uintptr_t va, int &to_decrease, size_t size,
                    uintptr_t fault_addr, unsigned long metadata)
        : request_base(paddr, va, to_decrease),
          size(size),
          fault_addr(fault_addr),
          metadata(metadata) {}
};

struct percpu_ctx_t {
    size_t issued;
    remote_queue queues;
    boost::circular_buffer<request_init> req_init;
    boost::circular_buffer<request_page> req_page;
    boost::circular_buffer<request_subpage> req_subpage;
    percpu_ctx_t()
        : issued(0),
          queues({max_req_init, max_req_page, max_req_subpage}, {}),
          req_init(max_req_init),
          req_page(max_req_page),
          req_subpage(max_req_subpage) {}
};

struct prefetch_stat_t {
   public:
    prefetch_stat_t(size_t entry_size) : entries(entry_size) {}
    struct entry_t {
        mmu::hw_ptep<0> ptep;
        uintptr_t va;
        entry_t(mmu::hw_ptep<0> ptep, uintptr_t va) : ptep(ptep), va(va) {}
    };

    inline void flush() { entries.clear(); }

    size_t hits(uintptr_t *va) {
        size_t h = 0;
        for (auto &entry : entries) {
            auto pte = entry.ptep.read();
            if (pte.valid() && pte.accessed()) {
                va[h] = entry.va;
                ++h;
            }
        }
        flush();
        return h;
    }

    inline void add(mmu::hw_ptep<0> ptep, uintptr_t va) {
        entries.push_back({ptep, va});
    }

   private:
    boost::circular_buffer<entry_t> entries;
};

struct perthread_ctx_t {
    int issued;
    prefetch_stat_t prefetch;
    perthread_ctx_t(size_t prefetch_size)
        : issued(0), prefetch(prefetch_size) {}
};

static std::array<percpu_ctx_t, max_cpu> percpu_ctx;

static thread_local perthread_ctx_t perthread_ctx(prefetch_window);

enum class fetch_type : uint8_t {
    INIT = 1,
    PAGE = 2,
    SUBPAGE = 3,
};

static inline uintptr_t embed_tag(uintptr_t offset, fetch_type tag) {
    static_assert(sizeof(uintptr_t) == 8);
    static_assert(sizeof(fetch_type) == 1);
    assert(!(offset >> 56));
    return offset | (static_cast<uintptr_t>(tag) << 56);
}
static inline fetch_type get_tag(uintptr_t token) {
    return static_cast<fetch_type>(token >> 56);
}
static inline uintptr_t get_offset(uintptr_t token) {
    return token & 0x00FFFFFFFFFFFFFFULL;
}

/* Event handler */
static bool do_hit_track = false;
static int nil_ev_handler(const struct ddc_event_t *event) { return 0; }
static ddc_handler_t ev_handler = nil_ev_handler;

struct ddc_event_impl_t {
    ddc_event_t inner;
    int *remain;
};
static_assert(offsetof(ddc_event_impl_t, inner) == 0);

/* Event handler End */

static void handle_init(unsigned my_cpu_id, uintptr_t va) {
    assert(!sched::preemptable());
    auto &ctx = percpu_ctx[my_cpu_id];
    auto &req = ctx.req_init.front();
    assert(req.va == va);
    auto before = protected_pte(req.ptep, my_cpu_id);
    auto after =
        make_leaf_pte(req.ptep, mmu::virt_to_phys(req.paddr), ddc_perm);
    after.set_accessed(true);
    after.set_dirty(false);
    trace_ddc_mmu_fault_remote_init_done(va, *(uint64_t *)req.paddr);
    auto &page = page_list.get_page(req.paddr, va, req.ptep);

    bool ret = req.ptep.compare_exchange(before, after);
    assert(ret);
    req.to_decrease -= 1;
    ctx.issued -= 1;
    ctx.req_init.pop_front();
    insert_page_buffered(page, false);
}

static void handle_page(unsigned my_cpu_id, uintptr_t va) {
    assert(!sched::preemptable());
    auto &ctx = percpu_ctx[my_cpu_id];
    auto &req = ctx.req_page.front();
    trace_ddc_mmu_polled_page(req.va, ctx.req_page.size());
    assert(req.va == va);
    auto before = protected_pte(req.ptep, my_cpu_id);
    auto after =
        make_leaf_pte(req.ptep, mmu::virt_to_phys(req.paddr), ddc_perm);
    after.set_accessed(false);
    after.set_dirty(false);
    trace_ddc_mmu_fault_remote_page_done(va, *(uint64_t *)req.paddr);
    auto &page = page_list.get_page(req.paddr, va, req.ptep);
    bool ret = req.ptep.compare_exchange(before, after);
    assert(ret);
    req.to_decrease -= 1;  // this is thread remain
    ctx.issued -= 1;
    ctx.req_page.pop_front();
    trace_ddc_mmu_polled_page_poped(va);
    insert_page_buffered(page, false);
}

static void handle_subpage(unsigned my_cpu_id, uintptr_t va) {
    assert(!sched::preemptable());
    auto &ctx = percpu_ctx[my_cpu_id];
    auto &req = ctx.req_subpage.front();

    // notify
    void *paddr = req.paddr;
    ddc_event_impl_t ev;
    ev.inner.type = ddc_event_t::DDC_EVENT_SUBPAGE_FETCHED;
    ev.inner.fault_addr = req.fault_addr;
    ev.inner.sub_page.addr = req.va;
    ev.inner.sub_page.sub_page = paddr;
    ev.inner.sub_page.size_of_sub_page = req.size;
    ev.inner.sub_page.metadata = req.metadata;
    ev.remain = &req.to_decrease;  // this is remain

    req.to_decrease -= 1;  // this is remain
    ctx.issued -= 1;
    ctx.req_subpage.pop_front();
    int handler_ret = ev_handler(&ev.inner);
    assert(handler_ret == 0);
    // now okay to free page

    DROP_LOCK(preempt_lock) {
        assert(sched::preemptable());
        memory::free_page(paddr);
    }
}
static int do_polling(unsigned my_cpu_id, int &remain) {
    assert(remain >= 0);
    assert(!sched::preemptable());

    auto &ctx = percpu_ctx[my_cpu_id];
    size_t to_poll = ctx.issued;
    uintptr_t tokens[to_poll];
    int polled = ctx.queues.poll(tokens, to_poll);
    trace_ddc_mmu_polled_size(polled);
    for (int i = 0; i < polled; ++i) {
        auto tag = get_tag(tokens[i]);
        auto offset = get_offset(tokens[i]);
        trace_ddc_mmu_polled((int)tag, offset);
        switch (tag) {
            case fetch_type::INIT:
                handle_init(my_cpu_id, get_offset(tokens[i]));
                break;
            case fetch_type::PAGE:
                handle_page(my_cpu_id, get_offset(tokens[i]));
                break;
            case fetch_type::SUBPAGE:
                handle_subpage(my_cpu_id, get_offset(tokens[i]));
                break;
            default:
                abort();
        }
    }
    try_flush_buffered();
    return polled;
}
class first_fault_handling {
   protected:
    bool do_page_small_first(mmu::pt_element<0> pte, mmu::hw_ptep<0> ptep,
                             uintptr_t va, void *paddr) {
        assert(paddr != NULL);
        memset(paddr, 0, mmu::page_size);  // TODO: no zerofill

        auto &page = page_list.get_page(paddr, va, ptep);

        auto pte_next = make_leaf_pte(ptep, mmu::virt_to_phys(paddr), ddc_perm);
        pte_next.set_accessed(true);

        while (true) {
            if (is_protected(pte)) {
                page_list.free_not_used_page(page);
                abort(
                    "UNIMPLEMENTED: wait protected one (fault handling first "
                    "access)");
            } else if (is_remote(pte)) {
                abort("unknwon remote fault");
            } else if (pte.valid()) {
                // This fault is handled by other thread.
                page_list.free_not_used_page(page);
                return true;
            }

            if (ptep.compare_exchange(pte, pte_next)) {
                // update success
                break;
            }

            pte = ptep.read();
        }
        WITH_LOCK(preempt_lock) { insert_page_buffered(page); }
        return true;
    }
};

// failback polling thread
struct polling_thread {
   public:
    polling_thread() : _trigger(false) {}
    void start(unsigned int cpu_id) {
        this->cpu_id = cpu_id;
        _thread = sched::thread::make(
            [&] { run(); },
            sched::thread::attr()
                .pin(sched::cpus[cpu_id])
                .name(osv::sprintf("polling_thread_%d", cpu_id)));
        _thread->start();
    }
    void wake() {
        _thread->wake_with(
            [&] { _trigger.store(true, std::memory_order_release); });
    }

   private:
    void run() {
        while (true) {
            sched::thread::wait_until([&] {
                return _trigger.load(std::memory_order_acquire) == true;
            });
            _trigger.store(false, std::memory_order_release);
            WITH_LOCK(preempt_lock) {
                int remain = 0;
                do_polling(cpu_id, remain);
            }
        }
    }
    unsigned int cpu_id;
    std::atomic_bool _trigger;
    sched::thread *_thread;
};

std::array<polling_thread, max_cpu> polling_threads;

class fault_handling
    : public mmu::vma_operation<mmu::allocate_intermediate_opt::yes,
                                mmu::skip_empty_opt::no, mmu::account_opt::no>,
      first_fault_handling {
   public:
#ifndef NO_STAT
    uint64_t __debug;
#endif
    fault_handling(uintptr_t fault_addr) : _fault_addr(fault_addr) {}
    bool page(mmu::hw_ptep<0> ptep, uintptr_t va) {
        auto pte = ptep.read();
        if (pte.valid()) {
            return true;
        }
        bool ret;
        if (is_first(pte)) {
            trace_ddc_mmu_fault_first(va);
            ret = do_page_small_first(pte, ptep, va, ::memory::alloc_page());
        } else {
            trace_ddc_mmu_fault_remote(va);
            ret = do_page_small_remote(pte, ptep, va);
            STAT_ELAPSED_TO(ddc_page_fault_remote, __debug);
        }
        return ret;
    }
    bool page(mmu::hw_ptep<1> ptep, uintptr_t va) {
        abort("not implemented: fault handling do_page_1\n");
        return true;
    }

   private:
    bool do_page_small_remote(mmu::pt_element<0> pte, mmu::hw_ptep<0> ptep,
                              uintptr_t va) {
        STAT_COUNTING(ddc_page_fault_remote);
#ifndef NORETURN_UNTIL_PREFETCH
        if (perthread_ctx.issued == 0) migration_lock.lock();
#else
        SCOPE_LOCK(migration_lock);
#endif
        unsigned my_cpu_id = sched::cpu::current()->id;
        int remain = 0;
        int poll_count = 0;
        // 1. Protect PTE to prevent duplicated fetching
        while (true) {
            if (pte.valid()) {
                // Handled by other thread
                return true;
            } else if (is_protected(pte)) {
                unsigned protector = get_cpu_id(pte);

                if (protector != my_cpu_id) {
                    STAT_COUNTING(ddc_page_fault_protected_other);
                    poll_count++;
                    // TODO: rare case polling thread
                    if (poll_count % 1000 == 0) {
                        polling_threads[protector].wake();
                    }
                }
                // fetching by other thread
                trace_ddc_mmu_fault_protected(protector);
                WITH_LOCK(preempt_lock) {
                    int polled = do_polling(my_cpu_id, remain);

                    if (!polled) sched::cpu::schedule();
                }

            } else if (!is_remote(pte)) {
                abort("unknwon fault 2");
            } else {
                auto protect_pte =
                    protected_pte(ptep,
                                  sched::cpu::current()->id);  // protect pte

                if (ptep.compare_exchange(pte, protect_pte)) {
                    break;
                }
            }
            pte = ptep.read();
        }

        // 2. alloc page to fetch
        void *paddr = ::memory::alloc_page();
        assert(paddr != NULL);
        trace_ddc_mmu_fault_remote_offset(va);
        // uintptr_t fault_page_addr = align_down(_fault_addr,
        // mmu::page_size);

        WITH_LOCK(preempt_lock) {
            auto &ctx = percpu_ctx[my_cpu_id];
            assert(!ctx.req_init.full());
#ifndef NO_SG
            bool ret =
                ctx.queues.fetch_vec(0, embed_tag(va, fetch_type::INIT), paddr,
                                     va_to_offset(va), pte.raw() >> 3);
#else
            bool ret =
                ctx.queues.fetch(0, embed_tag(va, fetch_type::INIT), paddr,
                                 va_to_offset(va), mmu::page_size);
#endif
            assert(ret);
            remain += 1;
            ctx.req_init.push_back({paddr, va, remain, ptep});
            ctx.issued += 1;

            // Inform prefetcher
            uintptr_t va[prefetch_window];
            ddc_event_impl_t ev;
            ev.inner.type = ddc_event_t::DDC_EVENT_PREFETCH_START;
            ev.inner.fault_addr = _fault_addr;
            ev.inner.start.hits =
                do_hit_track ? perthread_ctx.prefetch.hits(va) : 0;
            ev.inner.start.pages = va;
            ev.remain = &remain;
            int handler_ret = ev_handler(&ev.inner);
            assert(handler_ret == 0);
        }

        WITH_LOCK(preempt_lock) {
            while (remain > 0) {
                DROP_LOCK(preempt_lock) { sched::cpu::schedule(); }
                do_polling(my_cpu_id, remain);
            }
        }
#ifndef NORETURN_UNTIL_PREFETCH
        if (perthread_ctx.issued == 0) migration_lock.unlock();
#endif

        STAT_ELAPSED_TO(ddc_page_fault_remote_real, __debug);
        return true;
    }
    const uintptr_t _fault_addr;
};

template <typename T>
inline ulong ddc_operate_range(T mapper, void *start, size_t size) {
    return mmu::operate_range(mapper, NULL, reinterpret_cast<void *>(start),
                              size);
}

void vm_fault(uintptr_t addr, exception_frame *ef) {
    trace_ddc_mmu_fault(addr);
#ifndef NO_STAT
    auto token = STAT_ELAPSED_FROM();
#endif
    STAT_COUNTING(ddc_page_fault);
    uintptr_t fault_addr = addr;
    addr = align_down(addr, mmu::page_size);
    fault_handling pto(fault_addr);
#ifndef NO_STAT
    pto.__debug = token;
#endif
    ddc_operate_range(pto, reinterpret_cast<void *>(addr), mmu::page_size);
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd,
           off_t offset) {
    trace_ddc_mmu_mmap(addr, length, prot, flags);
    assert(!(flags & MAP_FIXED));
    assert(prot == (PROT_WRITE | PROT_READ) || prot == (PROT_NONE));
    assert(!(flags & MAP_POPULATE));
    assert(!(flags & MAP_FILE));
    assert(!(flags & MAP_SHARED));
    assert(flags & MAP_DDC);

    uintptr_t start = reinterpret_cast<uintptr_t>(addr);
    assert(addr == NULL || start == vma_middle);

    if (start == 0) {
        start = vma_start;
    }

    size_t mmap_align_bits = (flags >> MAP_ALIGNMENT_SHIFT) & 0xFF;
    mmap_align_bits = mmap_align_bits < 12 ? 12 : mmap_align_bits;
    size_t mmap_align = 1 << mmap_align_bits;

    // we do not check MAPED or UNMAPED
    // we trust jemalloc

    uintptr_t allocated = 0;
    bool special = false;

    {
        SCOPE_LOCK(mmu::vma_list_mutex.for_write());
        allocated = find_hole(start, length, mmap_align);

        if (flags & MAP_FIX_FIRST_TWO) {
            assert(length == 0x10000000);
            special = true;
            // MiMalloc Handling...
            constexpr size_t first_two_size = 2 * mmu::page_size;
            constexpr size_t segment_size = 0x400000;

            for (size_t i = 0; i < length; i += segment_size) {
                auto *front_vma = new anon_vma(
                    addr_range(allocated + i, allocated + i + first_two_size),
                    mmu::perm_rw,
                    mmu::mmap_fixed | mmu::mmap_small | mmu::mmap_populate);
                front_vma->set(allocated + i, allocated + i + first_two_size);
                vma_list.insert(*front_vma);
                mmu::populate_vma(front_vma, (void *)(allocated + i),
                                  first_two_size);
                auto *vma =
                    new mock_vma(addr_range(allocated + i + first_two_size,
                                            allocated + i + segment_size));
                vma->set(allocated + i + first_two_size,
                         allocated + i + segment_size);
                vma_list.insert(*vma);
            }

        } else {
            auto *vma = new mock_vma(addr_range(allocated, allocated + length));
            vma->set(allocated, allocated + length);
            vma_list.insert(*vma);
        }
    }

    if (special) allocated += 0x100000000000ul;

    assert(is_ddc(allocated));

    return reinterpret_cast<void *>(allocated);
}

static remote_queue syscall_queue({}, {max_syscall});

class pageout : public mmu::vma_operation<mmu::allocate_intermediate_opt::no,
                                          mmu::skip_empty_opt::yes,
                                          mmu::account_opt::yes>,
                public cleaner {
   public:
    static constexpr size_t max_free = 64;
    pageout()
        : cleaner(syscall_queue, 0, max_syscall),
          tlb_pushed(ddc::tlb_flusher::max_tokens),
          pushed(max_syscall),
          to_free(max_free) {}
    bool page(mmu::hw_ptep<0> ptep, uintptr_t va) {
        return page_inner(va, ptep, va);
    }

    bool page_inner(uintptr_t token, mmu::hw_ptep<0> ptep, uintptr_t va) {
        while (true) {
            auto ret = process(token, ptep, va_to_offset(va));
            switch (ret) {
                case state::OK_SKIP:
                case state::OK_FINISH:
                    return true;
                case state::OK_TLB_PUSHED:
                    tlb_pushed.push_back({ptep, va});
                    return true;
                case state::OK_PUSHED:
                    pushed.push_back({ptep, va});
                    return true;
                case state::PTE_UPDATE_FAIL:
                    continue;
                default:
                    abort("noreach");
            }
        }
    }
    bool page(mmu::hw_ptep<1> ptep, uintptr_t va) {
        abort("not implemented: evacuation do_page_huge\n");
        return true;
    }
    void tlb_flush_after(uintptr_t token) override {
        auto entry = tlb_pushed.front();
        assert(entry.va == token);
        tlb_pushed.pop_front();

        page_inner(token, entry.ptep, entry.va);
    }
    void push_after(uintptr_t token) override {
        auto entry = pushed.front();
        assert(entry.va == token);
        pushed.pop_front();
        page_inner(token, entry.ptep, entry.va);
    }

    void do_free() {
        SCOPE_LOCK(page_list_lock);
        for (mmu::phys paddr : to_free) {
            page_list.free_paddr_locked(paddr);
        }
        to_free.clear();
    }

    void try_free(mmu::phys paddr) {
        if (to_free.full()) do_free();

        to_free.push_back(paddr);
    }

    state clean_handler(uintptr_t token, mmu::hw_ptep<base_page_level> ptep,
                        mmu::pt_element<base_page_level> old,
                        uintptr_t offset) override {
        // evict!
        auto rpte = remote_pte_full(ptep);

        if (!ptep.compare_exchange(old, rpte)) {
            return state::PTE_UPDATE_FAIL;
        }

        // unmap!

        try_free(old.addr());

        return state::OK_FINISH;
    }
    void finalize(void) {
        while (!tlb_empty() || !cleaner_empty() || !to_free.empty()) {
            tlb_flush();
            poll_all();
            do_free();
        }
    }

   private:
    struct entry {
        mmu::hw_ptep<base_page_level> ptep;
        uintptr_t va;
    };

    boost::circular_buffer<entry> tlb_pushed;
    boost::circular_buffer<entry> pushed;
    boost::circular_buffer<mmu::phys> to_free;
};

// class pageout_sg : public
// mmu::vma_operation<mmu::allocate_intermediate_opt::no,
//                                              mmu::skip_empty_opt::yes,
//                                              mmu::account_opt::yes>,
//                    public cleaner {
//    public:
//     static constexpr size_t max_free = 64;
//     pageout_sg(uintptr_t mask)
//         : cleaner(syscall_queue, 0, max_syscall),
//           tlb_pushed(ddc::tlb_flusher::max_tokens),
//           pushed(max_syscall),
//           to_free(max_free),
//           _mask(mask) {}
//     bool page(mmu::hw_ptep<0> ptep, uintptr_t va) {
//         return page_inner(va, ptep, va);
//     }

//     bool page_inner(uintptr_t token, mmu::hw_ptep<0> ptep, uintptr_t va) {
//         while (true) {
//             auto ret = process(token, ptep, va_to_offset(va), _mask);
//             switch (ret) {
//                 case state::OK_SKIP:
//                 case state::OK_FINISH:
//                     return true;
//                 case state::OK_TLB_PUSHED:
//                     tlb_pushed.push_back({ptep, va});
//                     return true;
//                 case state::OK_PUSHED:
//                     pushed.push_back({ptep, va});
//                     return true;
//                 case state::PTE_UPDATE_FAIL:
//                     continue;
//                 default:
//                     abort("noreach");
//             }
//         }
//     }
//     bool page(mmu::hw_ptep<1> ptep, uintptr_t va) {
//         abort("not implemented: evacuation do_page_huge\n");
//         return true;
//     }
//     void tlb_flush_after(uintptr_t token) override {
//         auto entry = tlb_pushed.front();
//         assert(entry.va == token);
//         tlb_pushed.pop_front();

//         page_inner(token, entry.ptep, entry.va);
//     }
//     void push_after(uintptr_t token) override {
//         auto entry = pushed.front();
//         assert(entry.va == token);
//         pushed.pop_front();
//         page_inner(token, entry.ptep, entry.va);
//     }

//     void do_free() {
//         SCOPE_LOCK(page_list_lock);
//         for (mmu::phys paddr : to_free) {
//             page_list.free_paddr_locked(paddr);
//         }
//         to_free.clear();
//     }

//     void try_free(mmu::phys paddr) {
//         if (to_free.full()) do_free();

//         to_free.push_back(paddr);
//     }

//     state clean_handler(uintptr_t token, mmu::hw_ptep<base_page_level> ptep,
//                         mmu::pt_element<base_page_level> old, uintptr_t
//                         offset, uintptr_t mask) override {
//         // evict!
//         auto rpte = remote_pte_vec(ptep, mask);

//         if (!ptep.compare_exchange(old, rpte)) {
//             return state::PTE_UPDATE_FAIL;
//         }

//         // unmap!

//         try_free(old.addr());

//         return state::OK_FINISH;
//     }
//     void finalize(void) {
//         while (!tlb_empty() || !cleaner_empty() || !to_free.empty()) {
//             tlb_flush();
//             poll_all();
//             do_free();
//         }
//     }

//    private:
//     struct entry {
//         mmu::hw_ptep<base_page_level> ptep;
//         uintptr_t va;
//     };

//     boost::circular_buffer<entry> tlb_pushed;
//     boost::circular_buffer<entry> pushed;
//     boost::circular_buffer<mmu::phys> to_free;

//     uintptr_t _mask;
// };

class madv_freeing
    : public mmu::vma_operation<mmu::allocate_intermediate_opt::no,
                                mmu::skip_empty_opt::yes,
                                mmu::account_opt::yes>,
      public tlb_flusher {
   public:
    madv_freeing() : tlb_flusher() {}

    bool page(mmu::hw_ptep<0> ptep, uintptr_t va) {
        while (true) {
            auto pte = ptep.read();
            if (is_remote(pte)) {
                // if remote, just mark 0
                if (!ptep.compare_exchange(pte, make_empty_pte<0>())) {
                    continue;
                }
                return true;
            }
            if (is_protected(pte)) {
                // if protected (rarely), just giveup
                return true;
            }
            assert(pte.valid());

            auto new_pte = pte;
            new_pte.set_accessed(false);
            new_pte.set_dirty(false);
            // just mark not accessed and clean

            if (!ptep.compare_exchange(pte, new_pte)) {
                continue;
            }

            if (pte.accessed()) {
                // if was accessed, flush tlb
                tlb_push(va);
            }

            // non-zeroing might be okay for mimalloc

            return true;
        }

        return true;
    }

    bool page(mmu::hw_ptep<1> ptep, uintptr_t va) { return false; }

    void tlb_flush_after(uintptr_t token) override {
        // do nothing
    }
};

class madv_dontneed
    : public mmu::vma_operation<mmu::allocate_intermediate_opt::no,
                                mmu::skip_empty_opt::yes,
                                mmu::account_opt::yes>,
      public tlb_flusher {
   public:
    madv_dontneed() : tlb_flusher() {}

    bool page(mmu::hw_ptep<0> ptep, uintptr_t va) {
        mmu::pt_element<base_page_level> empty =
            make_empty_pte<base_page_level>();
        mmu::pt_element<base_page_level> old = ptep.exchange(empty);
        if (is_protected(old)) {
            // It's okay - fault handler will process this
        } else if (is_remote(old)) {
            // TODO: free remote

        } else if (old.valid()) {
            mmu::phys paddr = old.addr();
            tlb_push(paddr);

            return true;
        } else {
            debug_early_u64("pte: ", *(u64 *)&old);
            abort("noreach");
        }

        return true;
    }

    bool page(mmu::hw_ptep<1> ptep, uintptr_t va) { return false; }

    void tlb_flush_after(uintptr_t token) override {
        mmu::phys paddr = token;

        auto &page = ipt.lookup(paddr);
        assert(page.st != page_status::UNTRACK);

        if (page.st == page_status::TRACK_ACTIVE ||
            page.st == page_status::TRACK_CLEAN) {
            page_list.pop_page_locked(page);

            to_free.push_back(page);

        } else {
            // DONT_NEED
            // TRACK_PERCPU
            // TRACK_SLICE
            page.st = page_status::DONT_NEED;
        }
    }

   private:
    page_slice_t<base_page_level> to_free;
};

int madvise_dontneed(void *addr, size_t length) {
    // madv_dontneed pto;
    // WITH_LOCK(page_list_lock) { ddc_operate_range(pto, addr, length); }
    abort();
    return 0;
}
int madvise_free(void *addr, size_t length) {
    // do not need to hold syscall lock, no pushing
    madv_freeing pto;
    ddc_operate_range(pto, addr, length);
    return 0;
}

static mutex_t syscall_mutex;
int madvise_pageout(void *addr, size_t length) {
    SCOPE_LOCK(syscall_mutex);
    pageout pto;
    ddc_operate_range(pto, addr, length);

    return 0;
}

int madvise_pageout_sg(void *addr, size_t length, uint16_t mask) {
    SCOPE_LOCK(syscall_mutex);
    abort();
    //  FFFF FFFF FFFF FFFF
    // 0x1111111111111111
    // FFFF
    // if (!mask) return -1;
    // uint64_t expanded = _pdep_u64(mask, 0x1111111111111111) * 0xF;
    // pageout_sg pto(expanded);
    // ddc_operate_range(pto, addr, length);
    // return 0;
}
int madvise_print_stat() {
    size_t fetched_total = 0;
    size_t pushed_total = 0;
    size_t fetched = 0;
    size_t pushed = 0;

    for (size_t i = 0; i < sched::cpus.size(); ++i) {
        assert(i < max_cpu);

        fetched = 0;
        pushed = 0;

        percpu_ctx[i].queues.get_stat(fetched, pushed);
#ifdef PRINT_ALL
        debug_early_u64("CPU: ", i);
        debug_early_u64("  fetched: ", fetched);
        debug_early_u64("  pushed : ", pushed);
#endif

        fetched_total += fetched;
        pushed_total += pushed;
    }
    fetched = 0;
    pushed = 0;
    syscall_queue.get_stat(fetched, pushed);
#ifdef PRINT_ALL
    debug_early("Syscall:\n");
    debug_early_u64("  fetched: ", fetched);
    debug_early_u64("  pushed : ", pushed);
#endif
    fetched_total += fetched;
    pushed_total += pushed;

    fetched = 0;
    pushed = 0;
    eviction_get_stat(fetched, pushed);
#ifdef PRINT_ALL
    debug_early("Eviction:\n");
    debug_early_u64("  fetched: ", fetched);
    debug_early_u64("  pushed : ", pushed);
#endif

    fetched_total += fetched;
    pushed_total += pushed;
    printf("fetched: %ld pushed: %ld\n", fetched_total, pushed_total);

    return 0;
}

int madvise(void *addr, size_t length, int advice) {
    trace_ddc_mmu_madvise(addr, length, advice);
    int mask = advice & (~MADV_DDC_MASK);
    int advice2 = advice & (MADV_DDC_MASK);
    switch (advice2) {
        case MADV_DONTNEED:
            return madvise_dontneed(addr, length);
        case MADV_FREE:
            return madvise_free(addr, length);
        case MADV_DDC_PAGEOUT:
            return madvise_pageout(addr, length);
        case MADV_DDC_PAGEOUT_SG:
            return madvise_pageout_sg(addr, length, mask);
        case MADV_DDC_PRINT_STAT:
            return madvise_print_stat();
        default:
            printf("MADVISE FAIL: %d\n", advice);
            return -1;
    }
}

int mprotect(void *addr, size_t len, int prot) {
    trace_ddc_mmu_mprotect(addr, len, prot);
    assert(prot == (PROT_READ | PROT_WRITE));
    return 0;
}
int munmap(void *addr, size_t length) {
    trace_ddc_mmu_munmap(addr, length);
    // TODO: reuse address
    return 0;
}
int msync(void *addr, size_t length, int flags) {
    trace_ddc_mmu_msync(addr, length, flags);
    abort();
}
int mincore(void *addr, size_t length, unsigned char *vec) {
    trace_ddc_mmu_mincore(addr, length);
    abort();
}

class mlocking
    : public mmu::vma_operation<mmu::allocate_intermediate_opt::yes,
                                mmu::skip_empty_opt::no, mmu::account_opt::no>,
      first_fault_handling {
   public:
    static constexpr size_t max_free = 64;
    mlocking() {}
    bool page(mmu::hw_ptep<0> ptep, uintptr_t va) {
        auto pte = ptep.read();
        if (pte.valid()) {
            return page_inner(pte);
        }
        if (is_first(pte)) {
            DROP_LOCK(page_list_lock) {
                do_page_small_first(pte, ptep, va, ::memory::alloc_page());
            }
            pte = ptep.read();
            return page_inner(pte);
        } else {
            abort("not implemented: mlock remote page ");
        }
    }

    bool page_inner(mmu::pt_element<0> pte) {
        mmu::phys paddr = pte.addr();

        auto &page = ipt.lookup(paddr);

        switch (page.st) {
            case page_status::TRACK_ACTIVE:
                page.st = page_status::LOCKED;
                page_list.erase_from_active(page);
                break;
            case page_status::TRACK_CLEAN: {
                page.st = page_status::LOCKED;
                page_list.erase_from_clean(page);
                break;
            }
            case page_status::LOCKED:
                // do nothing
                break;
            case page_status::TRACK_PERCPU:
            case page_status::TRACK_SLICE:
                page.st = page_status::LOCKED;  // just mark locked, slicer
                                                // will lock
                break;
            default:
                abort("unknwon mlock");
        }

        return true;
    }

    bool page(mmu::hw_ptep<1> ptep, uintptr_t va) { return false; }
};

int mlock(const void *addr, size_t len) {
    trace_ddc_mmu_mlock(addr, len);
    SCOPE_LOCK(page_list_lock);
    mlocking pto;
    auto ret = ddc_operate_range(pto, (void *)addr, len);
    return ret;
}
int munlock(const void *addr, size_t len) {
    trace_ddc_mmu_munlock(addr, len);
    abort();
}

/* PREFETCH */

static inline void drop_and_free_page(void *page) {
    DROP_LOCK(preempt_lock) {
        assert(sched::preemptable());
        ::memory::free_page(page);
    }
}
class prefetching
    : public mmu::vma_operation<mmu::allocate_intermediate_opt::yes,
                                mmu::skip_empty_opt::no, mmu::account_opt::no>,
      first_fault_handling {
   public:
    prefetching(const ddc_event_impl_t &ev, const ddc_prefetch_t &command,
                void **page, ddc_prefetch_result_t &result)
        : ev(ev), command(command), _result(result), _page(page) {
        _result = DDC_RESULT_ERR_UNKNOWN;
    }
    bool page(mmu::hw_ptep<0> ptep, uintptr_t va) {
        auto pte = ptep.read();
        if (pte.valid()) {
            _result = DDC_RESULT_OK_LOCAL;
            // TODO: mark accessed

            auto sub_page_buff = get_sub_page_buffer();
            if (sub_page_buff.first) {
                mmu::phys paddr = pte.addr() + command.addr % memory::page_size;
                void *paddr_v = mmu::phys_to_virt(paddr);
                memcpy(sub_page_buff.first, paddr_v, sub_page_buff.second);
            }
            return true;
        }
        bool ret;
        if (is_first(pte)) {
            trace_ddc_mmu_prefetch_first(va);
            ret = do_page_small_first(pte, ptep, va, *_page);
            *_page = NULL;
            auto sub_page_buff = get_sub_page_buffer();
            if (sub_page_buff.first) {
                memset(sub_page_buff.first, 0, sub_page_buff.second);
            }
            _result = DDC_RESULT_OK_LOCAL;
        } else {
            trace_ddc_mmu_prefetch_remote(va);
            ret = do_page_small_remote(pte, ptep, va);
        }
        return ret;
    }
    bool page(mmu::hw_ptep<1> ptep, uintptr_t va) {
        abort("not implemented: fault handling do_page_1\n");
        return true;
    }

   private:
    inline std::pair<void *, size_t> get_sub_page_buffer() {
        void *buff = NULL;
        size_t length = 0;

        switch (command.type) {
            case ddc_prefetch_t::DDC_PREFETCH_SUBPAGE:
                buff = command.sub_page.buff;
                length = command.sub_page.length;
                break;

            case ddc_prefetch_t::DDC_PREFETCH_BOTH:
                buff = command.both.buff;
                length = command.both.length;
                break;

            default:
                break;
        }

        return std::make_pair(buff, length);
    }

    ddc_prefetch_result_t do_page_small_remote_page(mmu::pt_element<0> pte,
                                                    mmu::hw_ptep<0> ptep,
                                                    uintptr_t va) {
        trace_ddc_mmu_prefetch_remote_page(va);
        while (true) {  // there are three cases
            // 1. Protected case: pte is protected and is under processing
            // by other thread
            // 2. Remote case: pte is remote one.

            if (pte.valid()) {
                return DDC_RESULT_OK_PROCESSING;
            } else if (is_protected(pte)) {
                return DDC_RESULT_OK_PROCESSING;
            } else if (is_remote(pte)) {
                auto protect_pte = protected_pte(
                    ptep, sched::cpu::current()->id);  // protect pte

                if (ptep.compare_exchange(pte, protect_pte)) {
                    break;
                }
            } else {
                abort("unknown fault");
            }
            pte = ptep.read();
        }

        // use _page

        unsigned my_cpu_id = sched::cpu::current()->id;

        trace_ddc_mmu_prefetch_remote_offset(va, *_page);

        auto &ctx = percpu_ctx[my_cpu_id];
        assert(!ctx.req_init.full());
#ifndef NO_SG
        bool ret =
            ctx.queues.fetch_vec(1, embed_tag(va, fetch_type::PAGE), *_page,
                                 va_to_offset(va), pte.raw() >> 3);
#else
        bool ret = ctx.queues.fetch(1, embed_tag(va, fetch_type::PAGE), *_page,
                                    va_to_offset(va), mmu::page_size);
#endif
        assert(ret);

#ifndef NORETURN_UNTIL_PREFETCH
        // do return (no remain increase)
        perthread_ctx.issued += 1;
        ctx.req_page.push_back({*_page, va, perthread_ctx.issued, ptep});
#else
        *ev.remain += 1;
        ctx.req_page.push_back({*_page, va, *ev.remain, ptep});
#endif

        trace_ddc_mmu_prefetch_remote_offset_page(ctx.req_page.back().va,
                                                  *_page);
        ctx.issued += 1;
        *_page = NULL;
        if (do_hit_track) {
            perthread_ctx.prefetch.add(ptep, va);
        }
        return DDC_RESULT_OK_ISSUED;
    }
    ddc_prefetch_result_t do_page_small_remote_sub_page(mmu::pt_element<0> pte,
                                                        mmu::hw_ptep<0> ptep,
                                                        uintptr_t va) {
        trace_ddc_mmu_prefetch_remote_sub_page(va);

        while (pte.valid()) {
            // local
            mmu::phys paddr = pte.addr() + command.addr % memory::page_size;
            void *paddr_v = mmu::phys_to_virt(paddr);
            memcpy(command.sub_page.buff, paddr_v, command.sub_page.length);
            pte = ptep.read();
            if (pte.valid()) {
                return DDC_RESULT_OK_LOCAL;
            }
        }

        // remote

        unsigned my_cpu_id = sched::cpu::current()->id;
        uintptr_t offset = va_to_offset(command.addr);

        auto &ctx = percpu_ctx[my_cpu_id];
        bool ret =
            ctx.queues.fetch(2, embed_tag(command.addr, fetch_type::SUBPAGE),
                             *_page, offset, command.sub_page.length);

        assert(ret);
        // do return (no remain increase)
        *ev.remain += 1;
        ctx.req_subpage.push_back({*_page, command.addr, *ev.remain,
                                   command.sub_page.length, ev.inner.fault_addr,
                                   command.metadata});
        ctx.issued += 1;
        *_page = NULL;
        return DDC_RESULT_OK_ISSUED;
    }
    bool do_page_small_remote(mmu::pt_element<0> pte, mmu::hw_ptep<0> ptep,
                              uintptr_t va) {
        switch (command.type) {
            case ddc_prefetch_t::DDC_PREFETCH_PAGE:
                _result = do_page_small_remote_page(pte, ptep, va);
                break;
            case ddc_prefetch_t::DDC_PREFETCH_SUBPAGE:
                _result = do_page_small_remote_sub_page(pte, ptep, va);
                break;
            case ddc_prefetch_t::DDC_PREFETCH_BOTH:
                _result = do_page_small_remote_page(pte, ptep, va);
                // todo
                _result = do_page_small_remote_sub_page(pte, ptep, va);
                break;
            default:
                abort();
        }
        trace_ddc_mmu_prefetch_remote_result(_result);
        return true;
    }
    const ddc_event_impl_t &ev;
    const ddc_prefetch_t &command;
    ddc_prefetch_result_t &_result;
    void **_page;
};
/* PREFETCH END */

enum ddc_prefetch_result_t do_prefetch(const struct ddc_event_t *event,
                                       struct ddc_prefetch_t *command) {
    trace_ddc_mmu_prefetch_do_prefetch(command->addr);

    assert(!sched::preemptable());
    uintptr_t start =
        align_down(reinterpret_cast<uintptr_t>(command->addr), mmu::page_size);

    if (!ddc::is_ddc(start)) return DDC_RESULT_ERR_NOT_DDC;

    void *paddr = NULL;
    DROP_LOCK(preempt_lock) {
        assert(sched::preemptable());
        paddr = ::memory::alloc_page();
    }

    unsigned my_cpu_id = sched::cpu::current()->id;

    switch (command->type) {
        case ddc_prefetch_t::DDC_PREFETCH_PAGE:
            if (percpu_ctx[my_cpu_id].req_page.full()) {
                drop_and_free_page(paddr);
                return DDC_RESULT_ERR_QP_FULL;
            }
            break;
        case ddc_prefetch_t::DDC_PREFETCH_SUBPAGE:
            if (percpu_ctx[my_cpu_id].req_subpage.full()) {
                drop_and_free_page(paddr);
                return DDC_RESULT_ERR_QP_FULL;
            }
            assert(command->sub_page.length <= 128);
            assert(command->sub_page.length > 0);
            break;
        case ddc_prefetch_t::DDC_PREFETCH_BOTH:
            if (percpu_ctx[my_cpu_id].req_subpage.full() ||
                percpu_ctx[my_cpu_id].req_page.full()) {
                drop_and_free_page(paddr);
                return DDC_RESULT_ERR_QP_FULL;
            }
            assert(command->sub_page.length <= 128);
            assert(command->sub_page.length > 0);
            abort();
            break;

        default:
            abort("noreach");
    }

    ddc_prefetch_result_t result = DDC_RESULT_ERR_UNKNOWN;
    ddc::prefetching pto(*reinterpret_cast<const ddc_event_impl_t *>(event),
                         *command, &paddr, result);

    ddc_operate_range(pto, reinterpret_cast<void *>(command->addr),
                      mmu::page_size);

    if (paddr) {
        drop_and_free_page(paddr);
    }
    return result;
}

namespace options {
extern std::string prefetcher;
void *prefetcher_init_f = NULL;
}  // namespace options

static void *prefetcher_handle = NULL;
void prefetcher_init() {
    if (options::prefetcher.empty() || options::prefetcher == "no") {
        return;

    } else if (options::prefetcher == "readahead") {
        ddc_register_handler(handler_readahead, DDC_HANDLER_HITTRACK);
        return;
    } else if (options::prefetcher == "majority") {
        ddc_register_handler(handler_majority, DDC_HANDLER_HITTRACK);
        return;
    }

    auto prefetcher_path =
        std::string("/libprefetch_") + options::prefetcher + ".so";
    elf::register_skip_app(prefetcher_path);

    prefetcher_handle = dlopen(prefetcher_path.c_str(), 0);

    if (prefetcher_handle == NULL) {
        debug_early("Unable to load prefetcher: ");
        debug_early(prefetcher_path.c_str());
        debug_early("\n");
        return;
    }
    options::prefetcher_init_f =
        dlsym(prefetcher_handle, "ddc_prefetcher_init");
}

void ddc_init() {
    remote_init();
    reduce_total_memory(remote_reserve_size());

    for (size_t i = 0; i < sched::cpus.size(); ++i) {
        assert(i < max_cpu);
        percpu_ctx[i].queues.setup();
        polling_threads[i].start(i);
    }

    syscall_queue.setup();

    evictor_init();
    prefetcher_init();
    elf_replacer_init();
}

}  // namespace ddc

extern "C" {
enum ddc_prefetch_result_t ddc_prefetch(const struct ddc_event_t *event,
                                        struct ddc_prefetch_t *command) {
    return ddc::do_prefetch(event, command);
}
void ddc_register_handler(ddc_handler_t _handler, int option) {
    if (option & DDC_HANDLER_HITTRACK) {
        ddc::do_hit_track = true;
    }
    ddc::ev_handler = _handler;
}
}