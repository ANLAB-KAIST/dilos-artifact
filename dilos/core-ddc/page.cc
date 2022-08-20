#include "page.hh"

#include <osv/spinlock.h>

#include <ddc/memory.hh>
#include <ddc/phys.hh>
#include <osv/mmu.hh>
#include <osv/percpu.hh>
#include <osv/preempt-lock.hh>
#include <vector>
namespace ddc {

ipt_t<base_page_level> ipt(mmu::virt_to_phys(memory::get_phys_max()));

/* Page List */
spinlock_t page_list_lock;
page_list_t page_list;

static PERCPU(base_page_slice_t *, active_page_buffer);
static sched::cpu::notifier _notifier([]() {
    *active_page_buffer = new base_page_slice_t;
});
constexpr size_t active_list_buffer_size = 32;

void insert_page_buffered(base_page_t &page, bool try_flush) {
    (*active_page_buffer)->push_back(page);
    assert(page.ptep.read().addr() != NULL);
    if (try_flush) {
        try_flush_buffered();
    }
}

void try_flush_buffered() {
    if ((*active_page_buffer)->size() >= active_list_buffer_size) {
        base_page_slice_t temp_list;
        temp_list.splice(temp_list.end(), **active_page_buffer);
        DROP_LOCK(preempt_lock) { page_list.push_pages_active(temp_list); }
    }
}

}  // namespace ddc