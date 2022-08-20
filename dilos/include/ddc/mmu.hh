#pragma once

#include <cassert>

#include "exceptions.hh"
namespace ddc {

constexpr uintptr_t vma_start = 0x300000000000ul;
constexpr uintptr_t vma_middle = 0x400000000000ul;
constexpr uintptr_t vma_end = 0x500000000000ul;
constexpr uintptr_t offset_mask = 0x100000000000ul - 1;
constexpr uintptr_t auto_remote_size = (1ULL << 30);  // 1GB
constexpr size_t max_cpu = 24;

inline bool is_ddc(const uintptr_t addr) {
    return addr >= vma_start && addr < vma_end;
}
inline bool is_ddc(const void *addr) {
    return reinterpret_cast<uintptr_t>(addr) >= vma_start &&
           reinterpret_cast<uintptr_t>(addr) < vma_end;
}
inline uintptr_t va_to_offset(const uintptr_t va) {
    assert(is_ddc(va));
    return va & offset_mask;
}

void vm_fault(uintptr_t addr, exception_frame *ef);
int madvise(void *addr, size_t length, int advice);
void *mmap(void *addr, size_t length, int prot, int flags, int fd,
           off_t offset);
int mprotect(void *addr, size_t len, int prot);
int munmap(void *addr, size_t length);
int msync(void *addr, size_t length, int flags);
int mincore(void *addr, size_t length, unsigned char *vec);

int mlock(const void *addr, size_t len);
int munlock(const void *addr, size_t len);

void ddc_init();

}  // namespace ddc

#ifdef DDC
#define DDC_HANDLER(addr, handler, ...) \
    if (ddc::is_ddc(addr)) {            \
        return handler(__VA_ARGS__);    \
    }
#else
#define DDC_HANDLER(addr, handler, ...)
#endif