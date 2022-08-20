#pragma once

#include <osv/spinlock.h>

#include <ddc/memory.hh>
namespace ddc {

enum class page_status {
    UNTRACK,
    TRACK_PERCPU,
    TRACK_ACTIVE,
    TRACK_CLEAN,
    TRACK_SLICE,
    LOCKED,
    DONT_NEED
};

namespace bi = boost::intrusive;
template <int N>
struct page_t {
    uintptr_t va;
    uint64_t vec;  // this is for sg
    union {
        mmu::hw_ptep<N> ptep;
        void *meta;  // This is for app-driven metadata
    };
    bi::list_member_hook<> hook;

    // to read/modify this, needs active_list lock or initialization (ownership)
    page_status st;

    page_t() : va(0), vec(0), meta(NULL), st(page_status::UNTRACK) {}

    // this require ownership
    void reset() {
        va = 0;
        vec = 0;
        meta = NULL;
        st = page_status::UNTRACK;
    }
    page_t(const page_t &) = delete;
    page_t &operator=(const page_t &) = delete;
};
template <int N>
using page_slice_t = bi::list<
    page_t<N>,
    bi::member_hook<page_t<N>, bi::list_member_hook<>, &page_t<N>::hook>,
    bi::constant_time_size<true>>;

using base_page_t = page_t<base_page_level>;
using base_page_slice_t = page_slice_t<base_page_level>;

/* IPT */
template <unsigned level>
class ipt_t {
    const size_t page_size = mmu::page_size_level(level);

   public:
    ipt_t(mmu::phys end) : _end(end), pages(end / page_size) {
        assert(end % page_size == 0);
    }

    page_t<level> &lookup(mmu::phys addr) {
        assert(addr < _end);

        return pages[addr / page_size];
    }

    page_t<level> &lookup(void *addr) {
        return lookup(mmu::virt_to_phys(addr));
    }

    mmu::phys lookup(page_t<level> &page) {
        auto addr = (&page - pages.data()) * page_size;
        assert(addr < _end);
        return addr;
    }

   private:
    const mmu::phys _end;
    std::vector<page_t<level>> pages;
};

extern ipt_t<base_page_level> ipt;

extern spinlock_t page_list_lock;
class page_list_t {
   public:
    void check() {
        SCOPE_LOCK(page_list_lock);
        assert(active.size() + clean.size() != 0);
    }

    // used in first fault / remote fault
    base_page_t &get_page(void *paddr, uintptr_t va,
                          mmu::hw_ptep<base_page_level> &ptep) {
        base_page_t &pg = ipt.lookup(paddr);
        assert(pg.st == page_status::UNTRACK);
        pg.st = page_status::TRACK_PERCPU;
        pg.va = va;
        pg.ptep = ptep;
        assert(!pg.hook.is_linked());
        return pg;
    }

    void free_not_used_page(base_page_t &page) {
        page.st = page_status::UNTRACK;
        free_page(page);
    }

    void free_paddr_locked(mmu::phys paddr) {
        auto &page = ipt.lookup(paddr);
        assert(page.st != page_status::UNTRACK);

        if (page.st == page_status::TRACK_ACTIVE ||
            page.st == page_status::TRACK_CLEAN ||
            page.st == page_status::LOCKED) {
            pop_page_locked(page);
            free_not_used_page(page);

        } else {
            // DONT_NEED
            // TRACK_PERCPU
            // TRACK_SLICE
            page.st = page_status::DONT_NEED;
        }
    }

    void push_pages_active(base_page_slice_t &pages) {
        SCOPE_LOCK(page_list_lock);
        push_pages_inner(pages, active, page_status::TRACK_ACTIVE);
    }

    void slice_pages_active(base_page_slice_t &pages, size_t n) {
        slice_pages_inner(pages, n, active);
    }
    void slice_pages_clean(base_page_slice_t &pages, size_t n) {
        slice_pages_inner(pages, n, clean);
    }
    void push_pages_both(base_page_slice_t &pages_active,
                         base_page_slice_t &pages_dirty) {
        SCOPE_LOCK(page_list_lock);
        push_pages_inner(pages_active, active, page_status::TRACK_ACTIVE);
        push_pages_inner(pages_dirty, clean, page_status::TRACK_CLEAN);
    }
    void erase_from_active(base_page_t &page) {
        active.erase(active.iterator_to(page));
    }
    void erase_from_clean(base_page_t &page) {
        clean.erase(clean.iterator_to(page));
    }

    void pop_page_locked(base_page_t &page) {
        auto it = base_page_slice_t ::s_iterator_to(page);
        switch (page.st) {
            case page_status::TRACK_ACTIVE:
                active.erase(it);
                break;
            case page_status::TRACK_CLEAN:
                clean.erase(it);
                break;
            default:
                abort();
        }
    }

   private:
    void free_page(base_page_t &page) {
        assert(!page.hook.is_linked());
        assert(page.st == page_status::UNTRACK);
        auto paddr = ipt.lookup(page);
        void *vaddr = mmu::phys_to_virt(paddr);
        page.reset();
        ::memory::free_page(vaddr);
    }
    inline void push_pages_inner(base_page_slice_t &pages,
                                 base_page_slice_t &list, page_status st) {
        auto page = pages.begin();

        while (page != pages.end()) {
            if (page->st == page_status::DONT_NEED) {
                // this page marked need free;
                auto next = pages.erase(page);
                page->st = page_status::UNTRACK;
                free_page(*page);
                page = next;
            } else if (page->st == page_status::LOCKED) {
                // just move out
                page = pages.erase(page);
            } else {
                page->st = st;
                page++;
            }
        }
        list.splice(list.end(), pages);
    }

    void pop_page(base_page_t &page) {
        // TODO: check inpercpu
        SCOPE_LOCK(page_list_lock);
        pop_page_locked(page);
    }

    inline void slice_pages_inner(base_page_slice_t &pages, size_t n,
                                  base_page_slice_t &list) {
        SCOPE_LOCK(page_list_lock);
        size_t i;
        auto end = list.begin();
        for (i = 0; i < n && end != list.end(); ++i) {
            if (end->st == page_status::DONT_NEED) {
                auto next = list.erase(end);

                --i;
                free_page(*end);

                end = next;
            } else {
                end->st = page_status::TRACK_SLICE;
                ++end;
            }
        }
        if (i) {
            pages.splice(pages.end(), list, list.begin(), end, i);
        }
    }

   private:
    base_page_slice_t active;
    base_page_slice_t clean;
};
extern page_list_t page_list;

void insert_page_buffered(base_page_t &page,
                          bool try_flush = true);  // needs preempt_lock

void try_flush_buffered();  // needs preempt_lock

void eviction_get_stat(size_t &fetch_total, size_t &push_total);

}  // namespace ddc