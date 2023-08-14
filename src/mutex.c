
// SPDX-License-Identifier: MIT

#include "mutex.h"

#include "scheduler.h"
#include "time.h"

// Magic value for the magic field.
#define MUTEX_MAGIC     (int)0xcafebabe
// Magic value for exclusive locking.
#define EXCLUSIVE_MAGIC ((int)__INT_MAX__ / 2)



// Await the value of an `atomic_int`.
static bool await_atomic_int(atomic_int *var, timestamp_us_t timeout, int expected, int new_value, memory_order order) {
    do {
        int old_value = expected;
        if (atomic_compare_exchange_weak_explicit(var, &old_value, new_value, order, memory_order_relaxed)) {
            return true;
        } else {
            sched_yield();
        }
    } while (time_us() < timeout);
    return false;
}



// Initialise a mutex for unshared use.
void mutex_init(badge_err_t *ec, mutex_t *mutex) {
    if (atomic_load_explicit(&mutex->magic, memory_order_acquire) == MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
        return;
    }
    mutex->magic     = MUTEX_MAGIC;
    mutex->is_shared = false;
    atomic_store(&mutex->shares, 0);
    atomic_thread_fence(memory_order_release);
    badge_err_set_ok(ec);
}

// Initialise a mutex for shared use.
void mutex_init_shared(badge_err_t *ec, mutex_t *mutex) {
    if (atomic_load_explicit(&mutex->magic, memory_order_acquire) == MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
        return;
    }
    mutex->magic     = MUTEX_MAGIC;
    mutex->is_shared = true;
    atomic_store(&mutex->shares, 0);
    atomic_thread_fence(memory_order_release);
    badge_err_set_ok(ec);
}

// Clean up the mutex.
void mutex_destroy(badge_err_t *ec, mutex_t *mutex) {
    if (atomic_load_explicit(&mutex->magic, memory_order_acquire) != MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
        return;
    }
    mutex->magic = 0;
    atomic_thread_fence(memory_order_release);
}

// Try to acquire `mutex` within `timeout` microseconds.
// Returns true if the mutex was successully acquired.
bool mutex_acquire(badge_err_t *ec, mutex_t *mutex, timestamp_us_t timeout) {
    if (atomic_load_explicit(&mutex->magic, memory_order_acquire) != MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
        return false;
    }
    timeout += time_us();
    // Await the shared portion to reach 0 and then lock.
    if (await_atomic_int(&mutex->shares, timeout, 0, EXCLUSIVE_MAGIC, memory_order_acquire)) {
        // If that succeeds, the mutex was acquired.
        badge_err_set_ok(ec);
        return true;
    } else {
        // Acquire failed.
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_TIMEOUT);
        return false;
    }
}

// Release `mutex`, if it was initially acquired by this thread.
// Returns true if the mutex was successfully released.
bool mutex_release(badge_err_t *ec, mutex_t *mutex) {
    if (atomic_load_explicit(&mutex->magic, memory_order_acquire) != MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
        return false;
    }
    if (atomic_load(&mutex->shares) >= EXCLUSIVE_MAGIC) {
        atomic_fetch_sub_explicit(&mutex->shares, EXCLUSIVE_MAGIC, memory_order_release);
        badge_err_set_ok(ec);
        return true;
    } else {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
        return false;
    }
}

// Try to acquire a share in `mutex` within `timeout` microseconds.
// Returns true if the share was successfully acquired.
bool mutex_acquire_shared(badge_err_t *ec, mutex_t *mutex, timestamp_us_t timeout) {
    if (atomic_load_explicit(&mutex->magic, memory_order_acquire) != MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
        return false;
    }
    if (!mutex->is_shared) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
        return false;
    }
    timeout += time_us();
    // Take a share.
    int val = atomic_fetch_add_explicit(&mutex->shares, 1, memory_order_acquire);
    // Await the lock to be released.
    if (val < EXCLUSIVE_MAGIC) {
        // If that succeeds, the mutex was successfully acquired.
        badge_err_set_ok(ec);
        return true;
    } else {
        // If that fails, abort trying to lock.
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_TIMEOUT);
        atomic_fetch_sub_explicit(&mutex->shares, 1, memory_order_relaxed);
        return false;
    }
}

// Release `mutex`, if it was initially acquired by this thread.
// Returns true if the mutex was successfully released.
bool mutex_release_shared(badge_err_t *ec, mutex_t *mutex) {
    if (atomic_load_explicit(&mutex->magic, memory_order_acquire) != MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
        return false;
    }
    int old = atomic_fetch_sub_explicit(&mutex->shares, 1, memory_order_release);
    if (old == 0 || old == EXCLUSIVE_MAGIC) {
        // Prevent the counter from underflowing.
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
        atomic_fetch_add_explicit(&mutex->shares, 1, memory_order_relaxed);
        return false;
    } else {
        // Successful release.
        return true;
    }
}
