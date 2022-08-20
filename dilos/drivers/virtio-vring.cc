/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include <string.h>
#include <osv/mempool.hh>
#include <osv/mmu.hh>

#include "virtio.hh"
#include "drivers/virtio-vring.hh"
#include <osv/debug.hh>

#include <osv/sched.hh>
#include <osv/interrupt.hh>
#include "osv/trace.hh"
#include <osv/ilog2.hh>

using namespace memory;
using sched::thread;

TRACEPOINT(trace_virtio_enable_interrupts, "vring=%p", void*);
TRACEPOINT(trace_virtio_disable_interrupts, "vring=%p", void*);
TRACEPOINT(trace_virtio_kicked_event_idx,
                              "vring=%p kicking %d queue=%d "
                              "_avail %d _avail_event %d added_since_kick %d",
                              void*, bool, u16, u16, u16, u16);
TRACEPOINT(trace_virtio_add_buf, "vring=%p queue=%d, avail=%d",
                                 void*, u16, u16);

TRACEPOINT(trace_vring_get_buf_elem, "vring=%p _used_ring_host_head %d "
                                     "_used %d", void*, int, int);
TRACEPOINT(trace_vring_get_buf_gc, "vring=%p _used_ring_guest_head %d "
                                   "_used_ring_host_head %d", void*, int, int);
TRACEPOINT(trace_vring_get_buf_ret, "vring=%p _avail_count %d", void*, int);

namespace virtio {

    vring::vring(virtio_driver* const driver, u16 num, u16 q_index)
    {
        _driver = driver;
        _q_index = q_index;
        // Alloc enough pages for the vring...
        size_t alignment = driver->get_vring_alignment();
        size_t sz = VIRTIO_ALIGN(vring::get_size(num, alignment), alignment);
        _vring_ptr = memory::alloc_phys_contiguous_aligned(sz, 4096);
        memset(_vring_ptr, 0, sz);
        
        // Set up pointers        
        assert(is_power_of_two(num));
        _num = num;
        _desc = (vring_desc*)_vring_ptr;
        _avail = (vring_avail*)(_vring_ptr + num * sizeof(vring_desc));
        _used = (vring_used*)(((unsigned long)&_avail->_ring[num] +
                sizeof(u16) + alignment - 1) & ~(alignment - 1));

        // initialize the next pointer within the available ring
        for (int i = 0; i < num; i++) _desc[i]._next = i + 1;
        _desc[num-1]._next = 0;

        _cookie = new void*[num];

        _avail_head = 0;
        _used_ring_guest_head = 0;
        _used_ring_host_head = 0;
        _avail_added_since_kick = 0;
        _avail_count = num;

        _avail_event = reinterpret_cast<std::atomic<u16>*>(&_used->_used_elements[_num]);
        _used_event = reinterpret_cast<std::atomic<u16>*>(&_avail->_ring[_num]);

        _sg_vec.reserve(max_sgs);

        _use_indirect = false;
    }

    vring::~vring()
    {
        memory::free_phys_contiguous_aligned(_vring_ptr);
        delete [] _cookie;
    }

    u64 vring::get_paddr()
    {
        return mmu::virt_to_phys(_vring_ptr);
    }

    u64 vring::get_desc_addr()
    {
        return mmu::virt_to_phys(_desc);
    }

    u64 vring::get_avail_addr()
    {
        return mmu::virt_to_phys(_avail);
    }

    u64 vring::get_used_addr()
    {
        return mmu::virt_to_phys(_used);
    }

    unsigned vring::get_size(unsigned int num, unsigned long align)
    {
        return (((sizeof(vring_desc) * num + sizeof(u16) * (3 + num)
                 + align - 1) & ~(align - 1))
                + sizeof(u16) * 3 + sizeof(vring_used_elem) * num);
    }

    void vring::disable_interrupts()
    {
        trace_virtio_disable_interrupts(this);
        _avail->disable_interrupt();
    }

    inline bool vring::use_indirect(int desc_needed)
    {
        return _use_indirect &&
               _driver->get_indirect_buf_cap() &&
               // don't let the posting fail due to low available buffers number
               (desc_needed > _avail_count ||
               // no need to use indirect for a single descriptor
               (desc_needed > 1 &&
               // use indirect only when low space
               _avail_count < _num / 4));
    }

    void vring::enable_interrupts()
    {
        trace_virtio_enable_interrupts(this);
        _avail->enable_interrupt();
        set_used_event(_used_ring_host_head, std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_seq_cst);
    }

    bool
    vring::add_buf(void* cookie) {

            get_buf_gc();

            trace_virtio_add_buf(this, _q_index, _avail_count);

            int desc_needed = _sg_vec.size();
            bool indirect = false;
            if (use_indirect(desc_needed)) {
                desc_needed = 1;
                indirect = true;
            }

            if (_avail_count < desc_needed) {
                //make sure the interrupts get there
                //it probably should force an exit to the host
                kick();

                return false;
            }

            int idx, prev_idx = -1;
            idx = _avail_head;

            _cookie[idx] = cookie;
            vring_desc* descp = _desc;

            if (indirect) {
                vring_desc* indirect = reinterpret_cast<vring_desc*>(alloc_phys_contiguous_aligned((_sg_vec.size())*sizeof(vring_desc), 8));
                if (!indirect)
                    return false;
                _desc[idx]._flags = vring_desc::VRING_DESC_F_INDIRECT;
                _desc[idx]._paddr = mmu::virt_to_phys(indirect);
                _desc[idx]._len = (_sg_vec.size()) * sizeof(vring_desc);

                descp = indirect;
                //initialize the next pointers
                for (u32 j=0;j<_sg_vec.size();j++) descp[j]._next = j+1;
                //hack to make the logic below the for loop below act
                //just as before
                descp[_sg_vec.size()-1]._next = _desc[idx]._next;
                idx = 0;
            }

            for (unsigned i = 0; i < _sg_vec.size(); i++) {
                descp[idx]._flags = vring_desc::VRING_DESC_F_NEXT| _sg_vec[i]._flags;
                descp[idx]._paddr = _sg_vec[i]._paddr;
                descp[idx]._len = _sg_vec[i]._len;
                prev_idx = idx;
                idx = descp[idx]._next;
            }
            descp[prev_idx]._flags &= ~vring_desc::VRING_DESC_F_NEXT;

            _avail_added_since_kick++;
            _avail_count -= desc_needed;

            u16 avail_idx_cache = _avail->_idx.load(std::memory_order_relaxed);
            _avail->_ring[avail_idx_cache & (_num - 1)] = _avail_head;
            //Cheaper than the operator++ that uses seq consistency
            _avail->_idx.store(avail_idx_cache + 1, std::memory_order_release);
            _avail_head = idx;

            return true;
    }

    void
    vring::get_buf_gc()
    {
            vring_used_elem elem;

            trace_vring_get_buf_gc(this, _used_ring_guest_head,
                                   _used_ring_host_head);

            while (_used_ring_guest_head != _used_ring_host_head) {

                int i = 1;

                // need to trim the free running counter w/ the array size
                int used_ptr = _used_ring_guest_head & (_num - 1);

                elem = _used->_used_elements[used_ptr];
                int idx = elem._id;

                if (_desc[idx]._flags & vring_desc::VRING_DESC_F_INDIRECT) {
                    free_phys_contiguous_aligned(mmu::phys_to_virt(_desc[idx]._paddr));
                } else
                    while (_desc[idx]._flags & vring_desc::VRING_DESC_F_NEXT) {
                        idx = _desc[idx]._next;
                        i++;
                    }

                _used_ring_guest_head++;
                _avail_count += i;
                _desc[idx]._next = _avail_head; //instead, how about the end of the list?
                _avail_head = elem._id;  // what's the relation to the add_buf? can I just postpone this?
            }

            trace_vring_get_buf_ret(this, _avail_count);
    }


    void*
    vring::get_buf_elem(u32* len)
    {
            vring_used_elem elem;
            void* cookie = nullptr;

            // need to trim the free running counter w/ the array size
            int used_ptr = _used_ring_host_head & (_num - 1);
            u16 used_idx = _used->_idx.load(std::memory_order_acquire);

            trace_vring_get_buf_elem(this, _used_ring_host_head,
                                     used_idx);

            if (_used_ring_host_head == used_idx) {
                return nullptr;
            }

            elem = _used->_used_elements[used_ptr];
            *len = elem._len;

            cookie = _cookie[elem._id];
            _cookie[elem._id] = nullptr; //maybe use this array for the full size hdrs?

            return cookie;
    }

    bool vring::avail_ring_not_empty()
    {
        u16 effective_avail_count = effective_avail_ring_count();
        return effective_avail_count > 0;
    }

    bool vring::avail_ring_has_room(int descriptors)
    {
        u16 effective_avail_count = effective_avail_ring_count();
        if (use_indirect(descriptors))
            descriptors = 1;
        return effective_avail_count >= descriptors;
    }

    bool vring::used_ring_not_empty() const
    {
        return _used_ring_host_head != _used->_idx.load(std::memory_order_relaxed);
    }

    bool vring::used_ring_is_half_empty() const
    {
        return _used->_idx.load(std::memory_order_relaxed) - _used_ring_host_head > (u16)(_num / 2);
    }

    bool vring::used_ring_can_gc() const
    {
        return _used_ring_guest_head != _used_ring_host_head;
    }

    bool
    vring::kick() {
        bool kicked = true;

        if (_driver->get_event_idx_cap()) {

            std::atomic_thread_fence(std::memory_order_seq_cst);

            u16 avail_idx = _avail->_idx.load(std::memory_order_relaxed);
            u16 avail_event = _avail_event->load(std::memory_order_relaxed);

            kicked = ((u16)(avail_idx - avail_event - 1) <
                                                       _avail_added_since_kick);

            trace_virtio_kicked_event_idx(this, kicked, _q_index,
                    avail_idx, avail_event, _avail_added_since_kick);
        } else {
            std::atomic_thread_fence(std::memory_order_seq_cst);

            if (_used->notifications_disabled())
                return false;
        }

        //
        // Kick when the avail_event has moved or at least every half u16 range
        // packets since "kicked" above may loose an avail_event if it's update
        // is delayed for more than u16 range packets.
        //
        // Flushing every half range sounds like a feasible heuristics.
        // We don't want to flush at the levels close to the wrap around since
        // the call to kick() itself is not issued for every separate buffer
        // and _avail_added_since_kick might wrap around due to this bulking.
        //
        if (kicked || (_avail_added_since_kick >= (u16)(~0) / 2)) {
            _driver->kick(_q_index);
            _avail_added_since_kick = 0;
            return true;
        }

        return false;
    }

    void
    vring::add_buf_wait(void* cookie)
    {
        while (!add_buf(cookie)) {
            _waiter.reset(*sched::thread::current());
            while (!avail_ring_has_room(_sg_vec.size())) {
                sched::thread::wait_until([this] {return this->used_ring_can_gc();});
                get_buf_gc();
            }
            _waiter.clear();
        }
    }

};
