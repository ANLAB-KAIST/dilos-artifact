
#include <mimalloc.h>
#include <stdlib.h>

#include <ddc/elf.hh>
#include <ddc/mmu.hh>
#include <osv/debug.hh>

#include "init.hh"

#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) !!(x)
#define unlikely(x) !!(x)
#endif
extern "C" {
extern size_t malloc_usable_size(void *__ptr);
void *_ddc_malloc(size_t s) { return mi_malloc(s); }
void _ddc_free(void *ptr) {
    return ddc::is_ddc((uint64_t)ptr) ? mi_free(ptr) : free(ptr);
}

void *_ddc_calloc(size_t m, size_t n) { return mi_calloc(m, n); }

void *_ddc_aligned_alloc(size_t align, size_t len) {
    return mi_aligned_alloc(align, len);
}

void *_ddc_realloc(void *p, size_t n) {
    return ddc::is_ddc((uint64_t)p) ? mi_realloc(p, n) : realloc(p, n);
}

int _ddc_posix_memalign(void **res, size_t align, size_t len) {
    return mi_posix_memalign(res, align, len);
}

size_t _ddc_malloc_usable_size(void *obj) {
    return ddc::is_ddc((uint64_t)obj) ? mi_malloc_usable_size(obj)
                                      : malloc_usable_size(obj);
}
}

// CXX operators

__attribute__((noinline)) static void *handleOOM(std::size_t size,
                                                 bool nothrow) {
    void *ptr = nullptr;

    while (ptr == nullptr) {
        std::new_handler handler;
        // GCC-4.8 and clang 4.0 do not have std::get_new_handler.
        {
            static std::mutex mtx;
            std::lock_guard<std::mutex> lock(mtx);

            handler = std::set_new_handler(nullptr);
            std::set_new_handler(handler);
        }
        if (handler == nullptr) break;

        try {
            handler();
        } catch (const std::bad_alloc &) {
            break;
        }

        ptr = _ddc_malloc(size);
    }

    if (ptr == nullptr && !nothrow) std::__throw_bad_alloc();
    return ptr;
}

template <bool IsNoExcept>
__attribute__((always_inline)) inline void *newImpl(std::size_t size) noexcept(
    IsNoExcept) {
    void *ptr = _ddc_malloc(size);
    if (likely(ptr != nullptr)) return ptr;

    return handleOOM(size, IsNoExcept);
}

void *ddc_operator_new(std::size_t size) { return newImpl<false>(size); }

void *ddc_operator_new_array(std::size_t size) { return newImpl<false>(size); }

void *ddc_operator_new(std::size_t size, const std::nothrow_t &) noexcept {
    return newImpl<true>(size);
}

void *ddc_operator_new_array(std::size_t size,
                             const std::nothrow_t &) noexcept {
    return newImpl<true>(size);
}

void ddc_operator_delete(void *ptr) noexcept { _ddc_free(ptr); }

void ddc_operator_delete_array(void *ptr) noexcept { _ddc_free(ptr); }

void ddc_operator_delete(void *ptr, const std::nothrow_t &nt) noexcept {
    _ddc_free(ptr);
}

void ddc_operator_delete_array(void *ptr, const std::nothrow_t &nt) noexcept {
    _ddc_free(ptr);
}

#if __cpp_sized_deallocation >= 201309

void ddc_operator_delete(void *ptr, std::size_t size) noexcept {
    if (unlikely(ptr == nullptr)) {
        return;
    }
    abort("No support: cpp_sized_deallocation");
    // je_sdallocx_noflags(ptr, size);
}

void ddc_operator_delete_array(void *ptr, std::size_t size) noexcept {
    if (unlikely(ptr == nullptr)) {
        return;
    }
    abort("No support: cpp_sized_deallocation");
    // je_sdallocx_noflags(ptr, size);
}

#endif  // __cpp_sized_deallocation

namespace ddc {
void (*ddc_free)(void *) = NULL;
void malloc_init() {
    ddc_free = _ddc_free;
    REPLACEMENT_PREFIX(_ddc, malloc);
    REPLACEMENT_PREFIX(_ddc, free);
    REPLACEMENT_PREFIX(_ddc, calloc);
    REPLACEMENT_PREFIX(_ddc, aligned_alloc);
    REPLACEMENT_PREFIX(_ddc, realloc);
    REPLACEMENT_PREFIX(_ddc, posix_memalign);
    REPLACEMENT_PREFIX(_ddc, malloc_usable_size);

    elf::register_replacement(
        reinterpret_cast<void *>((void *(*)(size_t)) operator new),
        reinterpret_cast<void *>((void *(*)(size_t))ddc_operator_new), NULL);
    elf::register_replacement(
        reinterpret_cast<void *>(
            (void *(*)(size_t, const std::nothrow_t &)) operator new),
        reinterpret_cast<void *>(
            (void *(*)(size_t, const std::nothrow_t &))ddc_operator_new),
        NULL);
    elf::register_replacement(
        reinterpret_cast<void *>((void *(*)(size_t)) operator new[]),
        reinterpret_cast<void *>((void *(*)(size_t))ddc_operator_new_array),
        NULL);
    elf::register_replacement(
        reinterpret_cast<void *>(
            (void *(*)(size_t, const std::nothrow_t &)) operator new[]),
        reinterpret_cast<void *>(
            (void *(*)(size_t, const std::nothrow_t &))ddc_operator_new_array),
        NULL);

    elf::register_replacement(
        reinterpret_cast<void *>((void (*)(void *)) operator delete),
        reinterpret_cast<void *>((void (*)(void *))ddc_operator_delete), NULL);
    elf::register_replacement(
        reinterpret_cast<void *>(
            (void (*)(void *, const std::nothrow_t &)) operator delete),
        reinterpret_cast<void *>(
            (void (*)(void *, const std::nothrow_t &))ddc_operator_delete),
        NULL);
    elf::register_replacement(
        reinterpret_cast<void *>((void (*)(void *)) operator delete[]),
        reinterpret_cast<void *>((void (*)(void *))ddc_operator_delete_array),
        NULL);
    elf::register_replacement(
        reinterpret_cast<void *>(
            (void (*)(void *, const std::nothrow_t &)) operator delete[]),
        reinterpret_cast<void *>((
            void (*)(void *, const std::nothrow_t &))ddc_operator_delete_array),
        NULL);
#if __cpp_sized_deallocation >= 201309
    /* C++14's sized-delete operators. */
    elf::register_replacement(
        reinterpret_cast<void *>((void (*)(void *, size_t)) operator delete),
        reinterpret_cast<void *>((void (*)(void *, size_t))ddc_operator_delete),
        NULL);
    elf::register_replacement(
        reinterpret_cast<void *>((void (*)(void *, size_t)) operator delete[]),
        reinterpret_cast<void *>(
            (void (*)(void *, size_t))ddc_operator_delete_array),
        NULL);
#endif

    mi_option_disable(mi_option_large_os_pages);
    mi_option_disable(mi_option_reserve_huge_os_pages);
}
}  // namespace ddc