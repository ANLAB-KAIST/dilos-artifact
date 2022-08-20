#pragma once

#include <osv/spinlock.h>
#include <osv/sched.hh>
#include <string.h>

#include "compiler.h"
#include "atomic.h"
#include "assert.h"

#ifdef DEBUG
#define FORCE_INLINE inline
#else
#define FORCE_INLINE inline __attribute__((always_inline))
#endif

#define FOR_ALL_SOCKET0_CORES(core_id)                                         \
  for (uint32_t core_id = 0; core_id < helpers::kNumCPUs; core_id++)

#define CachelineAligned(type)                                                 \
  struct alignas(64) CachelineAligned_##type {                                 \
    type data;                                                                 \
  }


static inline int get_core_num(void) { return sched::cpu::current()->id; }


namespace helpers {

constexpr uint8_t kNumCPUs = 20;

static FORCE_INLINE uint32_t bsr_32(uint32_t a) {
  uint32_t ret;
  asm("BSR %k1, %k0   \n" : "=r"(ret) : "rm"(a));
  return ret;
}

static FORCE_INLINE uint64_t bsr_64(uint64_t a) {
  uint64_t ret;
  asm("BSR %q1, %q0   \n" : "=r"(ret) : "rm"(a));
  return ret;
}

static FORCE_INLINE constexpr uint32_t round_up_power_of_two(uint32_t a) {
  return a == 1 ? 1 : 1 << (32 - __builtin_clz(a - 1));
}

static FORCE_INLINE uint32_t bsf_32(uint32_t a) {
  uint32_t ret;
  asm("BSF %k1, %k0   \n" : "=r"(ret) : "rm"(a));
  return ret;
}

static FORCE_INLINE uint64_t bsf_64(uint64_t a) {
  uint64_t ret;
  asm("BSF %q1, %q0   \n" : "=r"(ret) : "rm"(a));
  return ret;
}

template <class F>
static FORCE_INLINE auto finally(F f) noexcept(noexcept(F(std::move(f)))) {
  auto x = [f = std::move(f)](void *) { f(); };
  return std::unique_ptr<void, decltype(x)>(reinterpret_cast<void *>(1),
                                            std::move(x));
}
}