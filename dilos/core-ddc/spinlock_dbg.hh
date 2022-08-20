#pragma once

#include <osv/spinlock.h>

template <int max_try>
struct spinlock_debug_t {
    spinlock_t _lock;
    inline void lock() {
        int try_ = 0;
        while (try_ < max_try) {
            if (spin_trylock(&_lock)) return;
            ++try_;
            if (try_ % 100 == 0) {
                debug_early_u64("_try: ", try_);
            }
        }
        abort();
    }
    inline void unlock() { _lock.unlock(); }
};
