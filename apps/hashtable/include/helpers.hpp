#pragma once

#include <osv/spinlock.h>
#include <cstdint>
#include <cassert>
#include <pthread.h>
#include <memory>
#include <sys/mman.h>

#ifdef DEBUG
#define FORCE_INLINE inline
#else
#define FORCE_INLINE inline __attribute__((always_inline))
#endif
#define __aligned(x) __attribute__((aligned(x)))
#define CACHE_LINE_SIZE	64
/**
 * __cstr - converts a value to a string
 */
#define __cstr_t(x...)	#x
#define __cstr(x...)	__cstr_t(x)


/* this helper trys to check a run-time assertion at built-time if possible */
#if !defined(__CHECKER__) && !defined(__cplusplus)
#define __build_assert_if_constant(cond)			\
	_Static_assert(__builtin_choose_expr(__builtin_constant_p(cond), \
		       (cond), true),				\
		       "run-time assertion caught at build-time")
#else /* __CHECKER__ */
#define __build_assert_if_constant(cond)
#endif /* __CHECKER__ */


#define BUG()							\
	do {							\
		__builtin_unreachable();			\
	} while (0)

/**
 * BUG_ON - a fatal check that doesn't compile out in release builds
 * @condition: the condition to check (fails on true)
 */
#define BUG_ON(cond)						\
	do {							\
		__build_assert_if_constant(!(cond));		\
		if (unlikely(cond)) {				\
			__builtin_unreachable();		\
		}						\
	} while (0)

#define FOR_ALL_SOCKET0_CORES(core_id)                                         \
  for (uint32_t core_id = 0; core_id < helpers::kNumCPUs; core_id++)
  
#define CachelineAligned(type)                                                 \
  struct alignas(64) CachelineAligned_##type {                                 \
    type data;                                                                 \
  }

#define DONT_OPTIMIZE(var) __asm__ __volatile__("" ::"m"(var));

#define	mb()	__asm __volatile("mfence;" : : : "memory")
#define	wmb()	__asm __volatile("sfence;" : : : "memory")
#define	rmb()	__asm __volatile("lfence;" : : : "memory")



#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif
#ifndef very_likely
// #define very_likely(x) __builtin_expect_with_probability(!!(x), 1, 1.0)
#define very_likely(x) likely(x)
#endif
#ifndef very_unlikely
// #define very_unlikely(x) __builtin_expect_with_probability(!!(x), 0, 1.0)
#define very_unlikely(x) unlikely(x)
#endif
#define unreachable() __builtin_unreachable()


#if !defined(__CHECKER__) && !defined(__cplusplus)
#define BUILD_ASSERT(cond) \
	_Static_assert(cond, "build-time condition failed")
#else /* __CHECKER__ */
#define BUILD_ASSERT(cond)
#endif /* __CHECKER__ */

#define barrier() asm volatile("" ::: "memory")

#define	ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))

/**
 * load_acquire - load a native value with acquire fence semantics
 * @p: the pointer to load
 */
#define load_acquire(p)				\
({						\
	BUILD_ASSERT(type_is_native(*p));	\
	typeof(*p) __p = ACCESS_ONCE(*p);	\
	barrier();				\
	__p;					\
})


namespace helpers {
constexpr uint8_t kPageShift = 12;
constexpr uint32_t kPageSize = (1 << kPageShift);
constexpr uint8_t kHugepageShift = 21;
constexpr uint32_t kHugepageSize = (1 << kHugepageShift);
constexpr uint8_t kNumCPUs = 20;
constexpr uint8_t kNumSocket1CPUs = 24;

static FORCE_INLINE constexpr uint32_t round_up_power_of_two(uint32_t a) {
  return a == 1 ? 1 : 1 << (32 - __builtin_clz(a - 1));
}


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

static FORCE_INLINE uint64_t round_to_hugepage_size(uint64_t size) {
  return ((size - 1) / helpers::kHugepageSize + 1) * helpers::kHugepageSize;
}


static FORCE_INLINE void *allocate_hugepage(uint64_t size) {
  size = round_to_hugepage_size(size);
  void *ptr = nullptr;
  // preempt_disable();
  int fail = posix_memalign(&ptr, kHugepageSize, size);
  BUG_ON(fail);
  BUG_ON(madvise(ptr, size, MADV_HUGEPAGE) != 0);
  // preempt_enable();
  return ptr;
}

static FORCE_INLINE constexpr size_t static_log(uint64_t b, uint64_t n) {
  return ((n < b) ? 1 : 1 + static_log(b, n / b));
}

}
static FORCE_INLINE void thread_yield(){
  pthread_yield();
}

extern int cycles_per_us;
extern uint64_t start_tsc; 


static inline uint64_t rdtsc(void)
{
	uint32_t a, d;
	asm volatile("rdtsc" : "=a" (a), "=d" (d));
	return ((uint64_t)a) | (((uint64_t)d) << 32);
}
static inline uint64_t rdtscp(uint32_t *auxp)
{
	uint32_t a, d, c;
	asm volatile("rdtscp" : "=a" (a), "=d" (d), "=c" (c));
	if (auxp)
		*auxp = c;
	return ((uint64_t)a) | (((uint64_t)d) << 32);
}

/**
 * microtime - gets the number of microseconds since the process started
 * This routine is very inexpensive, even compared to clock_gettime().
 */
static inline uint64_t microtime(void)
{
	return (rdtsc() - start_tsc) / cycles_per_us;
}
