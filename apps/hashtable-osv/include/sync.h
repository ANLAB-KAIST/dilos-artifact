#pragma once

#include <osv/sched.hh>
#include <cassert>

static inline void rep_nop(void)
{
    __asm__ __volatile__ ( "rep;nop" : : : "memory" );
}
#define cpu_relax() rep_nop()

namespace aifm{

typedef struct {
	volatile int locked;
} spinlock_t;

/**
 * spin_lock_init - prepares a spin lock for use
 * @l: the spin lock
 */
static inline void spin_lock_init(spinlock_t *l)
{
	l->locked = 0;
}

/**
 * spin_lock_held - determines if the lock is held
 * @l: the spin lock
 *
 * Returns true if the lock is held.
 */
static inline bool spin_lock_held(spinlock_t *l)
{
	return l->locked != 0;
}

/**
 * assert_spin_lock_held - asserts that the lock is currently held
 * @l: the spin lock
 */
static inline void assert_spin_lock_held(spinlock_t *l)
{
	assert(spin_lock_held(l));
}

/**
 * spin_lock - takes a spin lock
 * @l: the spin lock
 */
static inline void spin_lock(spinlock_t *l)
{
	while (__sync_lock_test_and_set(&l->locked, 1)) {
		while (l->locked)
			cpu_relax();
	}
}

/**
 * spin_try_lock- takes a spin lock, but only if it is available
 * @l: the spin lock
 *
 * Returns 1 if successful, otherwise 0
 */
static inline bool spin_try_lock(spinlock_t *l)
{
	if (!__sync_lock_test_and_set(&l->locked, 1))
		return true;
	return false;
}

/**
 * spin_unlock - releases a spin lock
 * @l: the spin lock
 */
static inline void spin_unlock(spinlock_t *l)
{
	assert_spin_lock_held(l);
	__sync_lock_release(&l->locked);
}
/*
 * Spin lock support
 */

/**
 * spin_lock_np - takes a spin lock and disables preemption
 * @l: the spin lock
 */
static inline void spin_lock_np(spinlock_t *l)
{
	sched::preempt_disable();
	spin_lock(l);
}

/**
 * spin_try_lock_np - takes a spin lock if its available and disables preemption
 * @l: the spin lock
 *
 * Returns true if successful, otherwise fail.
 */
static inline bool spin_try_lock_np(spinlock_t *l)
{
	sched::preempt_disable();
	if (spin_try_lock(l))
		return true;

	sched::preempt_enable();
	return false;
}

/**
 * spin_unlock_np - releases a spin lock and re-enables preemption
 * @l: the spin lock
 */
static inline void spin_unlock_np(spinlock_t *l)
{
	spin_unlock(l);
	sched::preempt_enable();
}


namespace rt {

// Spin lock support.
class Spin {
 public:
  Spin() { spin_lock_init(&lock_); }
  ~Spin() { assert(!spin_lock_held(&lock_)); }

  // Locks the spin lock.
  void Lock() { spin_lock_np(&lock_); }

  // Locks the spin lock, but with preemption enabled.
  void LockWp() { spin_lock(&lock_); }

  // Unlocks the spin lock.
  void Unlock() { spin_unlock_np(&lock_); }

  // Unlocks the spin lock, but with preemption enabled.
  void UnlockWp() { spin_unlock(&lock_); }

  // Locks the spin lock only if it is currently unlocked. Returns true if
  // successful, but with preemption enabled.
  bool TryLockWp() { return spin_try_lock(&lock_); }

  // Locks the spin lock only if it is currently unlocked. Returns true if
  // successful.
  bool TryLock() { return spin_try_lock_np(&lock_); }

 private:
  spinlock_t lock_;
  friend class CondVar;

  Spin(const Spin&) = delete;
  Spin& operator=(const Spin&) = delete;
};
}
}