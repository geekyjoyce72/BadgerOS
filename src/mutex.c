
// SPDX-License-Identifier: MIT

#include "mutex.h"

#include "scheduler.h"
#include "time.h"

#define MUTEX_MAGIC 0xfeca



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
    uint16_t magic = atomic_load_explicit(&mutex->magic, memory_order_acquire);
    if (magic == MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_INUSE);
    }
    atomic_store_explicit(&mutex->magic, MUTEX_MAGIC, memory_order_relaxed);
    atomic_store_explicit(&mutex->is_shared, false, memory_order_relaxed);
    atomic_store_explicit(&mutex->lock, 0, memory_order_relaxed);
    atomic_store_explicit(&mutex->shares, 0, memory_order_release);
    badge_err_set_ok(ec);
}

// Initialise a mutex for shared use.
void mutex_init_shared(badge_err_t *ec, mutex_t *mutex) {
    uint16_t magic = atomic_load_explicit(&mutex->magic, memory_order_acquire);
    if (magic == MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_INUSE);
    }
    atomic_store_explicit(&mutex->magic, MUTEX_MAGIC, memory_order_relaxed);
    atomic_store_explicit(&mutex->is_shared, true, memory_order_relaxed);
    atomic_store_explicit(&mutex->lock, 0, memory_order_relaxed);
    atomic_store_explicit(&mutex->shares, 0, memory_order_release);
    badge_err_set_ok(ec);
}

// Clean up the mutex.
void mutex_destroy(badge_err_t *ec, mutex_t *mutex) {
    uint16_t magic = atomic_load_explicit(&mutex->magic, memory_order_acquire);
    if (magic != MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
    }
    atomic_store_explicit(&mutex->magic, 0, memory_order_release);
}

// Try to acquire `mutex` within `timeout` microseconds.
// Returns true if the mutex was successully acquired.
bool mutex_acquire(badge_err_t *ec, mutex_t *mutex, timestamp_us_t timeout) {
    uint16_t magic = atomic_load_explicit(&mutex->magic, memory_order_acquire);
    if (magic != MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
    }
    timeout += time_us();
    // Take the lock.
    if (!await_atomic_int(&mutex->lock, timeout, false, true, memory_order_acquire)) {
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
        atomic_store_explicit(&mutex->lock, false, memory_order_release);
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_TIMEOUT);
        return false;
    }
}

// Release `mutex`, if it was initially acquired by this thread.
// Returns true if the mutex was successfully released.
bool mutex_release(badge_err_t *ec, mutex_t *mutex) {
    uint16_t magic = atomic_load_explicit(&mutex->magic, memory_order_acquire);
    if (magic != MUTEX_MAGIC) {
        badge_err_set(ec, ELOC_UNKNOWN, ECAUSE_ILLEGAL);
    }
    int the_true = true;
    if (atomic_compare_exchange_strong_explicit(
            &mutex->lock,
            &the_true,
            false,
            memory_order_release,
            memory_order_relaxed
        )) {
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
    uint16_t magic = atomic_load_explicit(&mutex->magic, memory_order_acquire);
    if (magic != MUTEX_MAGIC) {
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
    if (await_atomic_int(&mutex->lock, timeout, false, false, memory_order_acquire)) {
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
    uint16_t magic = atomic_load_explicit(&mutex->magic, memory_order_acquire);
    if (magic != MUTEX_MAGIC) {
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
