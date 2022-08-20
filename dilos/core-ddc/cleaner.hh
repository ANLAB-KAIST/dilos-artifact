#pragma once

#include <ddc/memory.hh>
#include <ddc/mmu.hh>
#include <ddc/remote.hh>

#include "pte.hh"
#include "tlb.hh"

namespace ddc {

class cleaner : public tlb_flusher {
   private:
    bool check_skip(mmu::pt_element<base_page_level> &pte) {
        if (is_protected(pte)) {
            // It's okay - fault handler will process this
            return true;
        } else if (is_remote(pte)) {
            // It's okay - already remote
            return true;
        }
        return false;
    }

   public:
    enum class state {
        OK_SKIP,
        OK_TLB_PUSHED,
        OK_PUSHED,
        OK_FINISH,
        PTE_UPDATE_FAIL,
    };

    cleaner(remote_queue &rq, int qp_id, const size_t max_push)
        : _rq(rq), pushed(0), _qp_id(qp_id), _max_push(max_push) {}

    state process(uintptr_t token, mmu::hw_ptep<base_page_level> ptep,
                  uintptr_t offset) {
        mmu::pt_element<base_page_level> old = ptep.read();
        if (check_skip(old)) {
            // It's okay - fault handler will process this
            return state::OK_SKIP;
        }
        if (old.valid()) {
            if (old.accessed()) {
                return accessed_handler(token, ptep, old, offset);
            }
            if (old.dirty()) {
                return dirty_handler(token, ptep, old, offset);
            } else {
                return clean_handler(token, ptep, old, offset);
            }
        }

        abort("noreach\n");
    }

    state process_mask(uintptr_t token, mmu::hw_ptep<base_page_level> ptep,
                       uintptr_t offset, uintptr_t va, uintptr_t &vec) {
        mmu::pt_element<base_page_level> old = ptep.read();
        if (check_skip(old)) {
            // It's okay - fault handler will process this
            return state::OK_SKIP;
        }
        if (old.valid()) {
            if (old.accessed()) {
                return accessed_handler(token, ptep, old, offset);
            }
            if (old.dirty()) {
                return dirty_handler(token, ptep, old, offset, va, vec);
            } else {
                return clean_handler(token, ptep, old, offset, va, vec);
            }
        }

        abort("noreach\n");
    }

    state accessed_handler(uintptr_t token, mmu::hw_ptep<base_page_level> ptep,
                           mmu::pt_element<base_page_level> old,
                           uintptr_t offset) {
        auto new_pte = old;
        new_pte.set_accessed(false);
        if (!ptep.compare_exchange(old, new_pte)) {
            return state::PTE_UPDATE_FAIL;
        }
        tlb_push(token);
        return state::OK_TLB_PUSHED;
    }

    state dirty_handler_inner(uintptr_t token,
                              mmu::pt_element<base_page_level> old,
                              uintptr_t offset, uintptr_t vec) {
        mmu::phys paddr = old.addr();

        while (pushed >= _max_push) {
            poll_until_one();
        }
        _rq.push_vec(_qp_id, token, mmu::phys_to_virt(paddr), offset, vec);
        ++pushed;

        return state::OK_PUSHED;
    }
    state dirty_handler(uintptr_t token, mmu::hw_ptep<base_page_level> ptep,
                        mmu::pt_element<base_page_level> old,
                        uintptr_t offset) {
        // push
        auto new_pte = old;
        new_pte.set_dirty(false);
        if (!ptep.compare_exchange(old, new_pte)) {
            return state::PTE_UPDATE_FAIL;
        }
        mmu::phys paddr = old.addr();

        while (pushed >= _max_push) {
            poll_until_one();
        }
        _rq.push(_qp_id, token, mmu::phys_to_virt(paddr), offset,
                 base_page_size);
        ++pushed;

        return state::OK_PUSHED;
    }
    state dirty_handler(uintptr_t token, mmu::hw_ptep<base_page_level> ptep,
                        mmu::pt_element<base_page_level> old, uintptr_t offset,
                        uintptr_t va, uintptr_t &vec) {
        // push
        auto new_pte = old;
        new_pte.set_dirty(false);
        if (!ptep.compare_exchange(old, new_pte)) {
            return state::PTE_UPDATE_FAIL;
        }
        vec = get_vec(va);
        return dirty_handler_inner(token, old, offset, vec);
    }

    virtual uint64_t get_vec(uintptr_t va) {
        abort("unimplemented\n");
        return 0;
    }

    virtual void push_after(uintptr_t token) = 0;

    virtual state clean_handler(uintptr_t token,
                                mmu::hw_ptep<base_page_level> ptep,
                                mmu::pt_element<base_page_level> old,
                                uintptr_t offset) {
        abort("call unimpled clean handler");
    }

    virtual state clean_handler(uintptr_t token,
                                mmu::hw_ptep<base_page_level> ptep,
                                mmu::pt_element<base_page_level> old,
                                uintptr_t offset, uintptr_t va,
                                uintptr_t &vec) {
        abort("call unimpled clean (mask) handler");
    }

    inline bool cleaner_empty() { return !pushed; }

    size_t poll_once() {
        if (cleaner_empty()) return 0;
        uintptr_t tokens[pushed];
        int polled = _rq.poll(tokens, pushed);
        for (int i = 0; i < polled; ++i) {
            push_after(tokens[i]);
        }
        assert(pushed != 224);
        pushed -= polled;
        assert(pushed >= 0);
        return polled;
    }

    size_t poll_until_one() {
        assert(sched::preemptable());
        size_t polled = poll_once();
        while (!polled) {
            sched::cpu::schedule();
            polled = poll_once();
        }
        return polled;
    }

    size_t poll_all() {
        assert(sched::preemptable());
        size_t polled = poll_once();
        while (pushed) {
            sched::cpu::schedule();
            polled += poll_once();
        }
        return polled;
    }

   private:
    remote_queue &_rq;
    size_t pushed;
    int _qp_id;
    const size_t _max_push;
};

}  // namespace ddc