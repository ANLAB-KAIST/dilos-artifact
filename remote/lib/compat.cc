
#include <linux/mman.h>
#include <osv/spinlock.h>
#include <sys/mman.h>

#include <cassert>
#include <ddc/debug.hh>
#include <ddc/options.hh>
#include <ddc/phys.hh>
#include <ddc/remote.hh>
#include <osv/barrier.hh>

void ddc_debug(const char *text, uint64_t val) { printf("%s%lx\n", text, val); }

namespace ddc {
namespace options {
std::string ib_device;
int ib_port;
std::string ms_ip;
int ms_port;
int gid_idx;
std::string prefetcher;
bool no_eviction;

}  // namespace options
}  // namespace ddc

namespace memory {

static std::array<std::pair<void *, size_t>, max_phys_list> phys_list;
static uint8_t *simulated_phy_memory = NULL;
static uint8_t *simulated_phy_memory_start = NULL;
static size_t simulated_phy_memory_size = 0;

const std::array<std::pair<void *, size_t>, max_phys_list> &get_phys_list(
    size_t &n) {
    n = 1;
    return phys_list;
}
void non_osv_init(std::string ib_device, int ib_port, std::string ms_ip,
                  int ms_port, int gid_idx, size_t phy_mem) {
    ddc::options::ib_device = ib_device;
    ddc::options::ib_port = ib_port;
    ddc::options::ms_ip = ms_ip;
    ddc::options::ms_port = ms_port;
    ddc::options::gid_idx = gid_idx;
    assert(simulated_phy_memory == NULL);
    simulated_phy_memory =
        (uint8_t *)mmap(NULL, phy_mem, PROT_WRITE | PROT_READ,
                        MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
    simulated_phy_memory_start = simulated_phy_memory;
    printf("simulated_phy_memory: %p\n", simulated_phy_memory);

    assert(simulated_phy_memory != MAP_FAILED);
    simulated_phy_memory_size = phy_mem;

    phys_list[0].first = simulated_phy_memory_start;
    phys_list[0].second = simulated_phy_memory_size;
}

void *alloc_page() {
    assert(simulated_phy_memory != NULL);
    void *ret = (void *)simulated_phy_memory;
    simulated_phy_memory += 4096;
    assert(simulated_phy_memory - simulated_phy_memory_start <=
           simulated_phy_memory_size);
    return ret;
}

}  // namespace memory

void spin_lock(spinlock_t *sl) {
    while (__sync_lock_test_and_set(&sl->_lock, 1)) {
        while (sl->_lock) {
            barrier();
        }
    }
}

bool spin_trylock(spinlock_t *sl) {
    if (__sync_lock_test_and_set(&sl->_lock, 1)) {
        return false;
    }
    return true;
}

void spin_unlock(spinlock_t *sl) { __sync_lock_release(&sl->_lock, 0); }
