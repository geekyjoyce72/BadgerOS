
// SPDX-License-Identifier: MIT

#include "mutex.h"

#include "scheduler.h"
#include "time.h"

#define MUTEX_MAGIC 0xfeca



// Try to set an atomic flag.
static bool await_set_atomic_flag(atomic_flag *var, timestamp_us_t timeout) {
    do {
        if (atomic_flag_test_and_set_explicit(var, memory_order_release)) {
            sched_yield();
        } else {
            return true;
        }
    } while (time_us() < timeout);
    return false;
}

// Await the clearing of an atomic flag.
static bool await_clear_atomic_flag(atomic_flag *var, timestamp_us_t timeout) {
    do {
        if (atomic_flag_test_and_set_explicit(var, memory_order_acquire)) {
            sched_yield();
        } else {
            atomic_flag_clear_explicit(var, memory_order_seq_cst);
            return true;
        }
    } while (time_us() < timeout);
    return false;
}

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
    if (mutex->magic == MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_INUSE);
    }
    mutex->magic     = MUTEX_MAGIC;
    mutex->is_shared = false;
    mutex->lock      = (atomic_flag)ATOMIC_FLAG_INIT;
    atomic_store(&mutex->shares, 0);
    atomic_thread_fence(memory_order_release);
    badge_err_set_ok(ec);
}

// Initialise a mutex for shared use.
void mutex_init_shared(badge_err_t *ec, mutex_t *mutex) {
    if (mutex->magic == MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_INUSE);
    }
    mutex->magic     = MUTEX_MAGIC;
    mutex->is_shared = true;
    atomic_flag_clear(&mutex->lock);
    atomic_store(&mutex->shares, 0);
    atomic_thread_fence(memory_order_release);
    badge_err_set_ok(ec);
}

// Clean up the mutex.
void mutex_destroy(badge_err_t *ec, mutex_t *mutex) {
    if (mutex->magic != MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
    }
    mutex->magic = 0;
    atomic_thread_fence(memory_order_release);
}

// Try to acquire `mutex` within `timeout` microseconds.
// Returns true if the mutex was successully acquired.
bool mutex_acquire(badge_err_t *ec, mutex_t *mutex, timestamp_us_t timeout) {
    if (mutex->magic != MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
    }
    timeout += time_us();
    // Take the lock.
    if (!await_set_atomic_flag(&mutex->lock, timeout)) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_TIMEOUT);
        return false;
    }
    // Await the shared portion to reach 0.
    if (await_atomic_int(&mutex->shares, timeout, 0, 0, memory_order_acquire)) {
        // If that succeeds, the mutex was acquired.
        badge_err_set_ok(ec);
        return true;
    } else {
        // If that fails, abort trying to lock.
        atomic_flag_clear_explicit(&mutex->lock, memory_order_release);
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_TIMEOUT);
        return false;
    }
}

// Release `mutex`, if it was initially acquired by this thread.
// Returns true if the mutex was successfully released.
bool mutex_release(badge_err_t *ec, mutex_t *mutex) {
    if (mutex->magic != MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
    }
    if (atomic_flag_test_and_set(&mutex->lock)) {
        atomic_flag_clear_explicit(&mutex->lock, memory_order_release);
        badge_err_set_ok(ec);
        return true;
    } else {
        atomic_flag_clear_explicit(&mutex->lock, memory_order_release);
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
        return false;
    }
}

// Try to acquire a share in `mutex` within `timeout` microseconds.
// Returns true if the share was successfully acquired.
bool mutex_acquire_shared(badge_err_t *ec, mutex_t *mutex, timestamp_us_t timeout) {
    if (mutex->magic != MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
    }
    if (!mutex->is_shared) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
        return false;
    }
    timeout += time_us();
    // Take a share.
    atomic_fetch_add_explicit(&mutex->shares, 1, memory_order_acquire);
    // Await the lock to be released.
    if (await_clear_atomic_flag(&mutex->lock, timeout)) {
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
    if (mutex->magic != MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
    }
    int old = atomic_fetch_sub_explicit(&mutex->shares, 1, memory_order_release);
    if (old <= 0) {
        // Prevent the counter from going below 0.
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
        atomic_fetch_add_explicit(&mutex->shares, 1, memory_order_relaxed);
        return false;
    } else {
        // Successful release.
        return true;
    }
}
