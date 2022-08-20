#include <osv/debug.h>

#include <atomic>
#include <cassert>
#include <ddc/alloc.hh>
#include <osv/mmu.hh>

namespace ddc {
namespace alloc {

static std::atomic_uintptr_t bf_start(NULL);
static uintptr_t bf_end(NULL);

bool pre_init(void *addr, size_t len) {
    if (bf_start.load() != NULL || len != 0x100000) return false;

    uintptr_t addr_v = (uintptr_t)addr;
    uintptr_t end_v = addr_v + len;

    addr_v = (addr_v + (mmu::page_size - 1)) & -mmu::page_size;
    end_v = ((end_v >> mmu::page_size_shift) << mmu::page_size_shift);
    bf_start.store(addr_v);
    bf_end = end_v;
    return true;
}
void *pre_alloc_buffer(size_t size) {
    assert(size % mmu::page_size == 0);
    auto addr = bf_start.fetch_add(size);

    assert(addr != NULL && addr < bf_end);

    return (void *)addr;
}
void pre_free_page(void *ptr) { assert(0); }

}  // namespace alloc

}  // namespace ddc