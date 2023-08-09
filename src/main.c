
// SPDX-License-Identifier: MIT

#include "assertions.h"
#include "gpio.h"
#include "log.h"
#include "malloc.h"
#include "memprotect.h"
#include "port/interrupt.h"
#include "rawprint.h"
#include "scheduler.h"
#include "time.h"

#include <stdint.h>

// Temporary kernel context until threading is implemented.
static kernel_ctx_t kctx;

void                debug_func(void *);
#define stack_size 1024
static uint8_t stack0[stack_size] ALIGNED_TO(STACK_ALIGNMENT);

// This is the entrypoint after the stack has been set up and the init functions
// have been run. Main is not allowed to return, so declare it noreturn.
void main() NORETURN;
void main() {
    badge_err_t err;

    // Install interrupt and trap handlers.
    interrupt_init(&kctx);

    // Set up memory protection.
    memprotect_init();

    // Set up timers and watchdogs.
    // This function must run within the first ~1s of power-on time and should be
    // called as early as possible.
    time_init();

    // Set up memory allocator.
    kernel_heap_init();

    // Set up multithreading.
    sched_init(&err);
    assert_always(badge_err_is_ok(&err));

    // A debug thread.
    sched_thread_t *const debug_thread_0 =
        sched_create_kernel_thread(&err, debug_func, NULL, stack0, stack_size, SCHED_PRIO_NORMAL);
    assert_always(badge_err_is_ok(&err));
    sched_set_name(&err, debug_thread_0, "debug0");
    assert_always(badge_err_is_ok(&err));
    sched_resume_thread(&err, debug_thread_0);
    assert_always(badge_err_is_ok(&err));

    // Enter the scheduler
    sched_exec();
    __builtin_unreachable();
}

void debug_func(void *arg) {
    (void)arg;
    io_mode(NULL, 19, IO_MODE_OUTPUT);
    timestamp_us_t t;
    while (1) {
        io_write(NULL, 19, true);
        t = time_us() + 500000;
        while (time_us() < t) sched_yield();
        io_write(NULL, 19, false);
        t = time_us() + 500000;
        while (time_us() < t) sched_yield();
    }
}
