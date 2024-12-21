
// SPDX-License-Identifier: MIT

#pragma once

#include "time.h"

// Timer callback.
typedef struct {
    // Timer task no.
    int64_t        taskno;
    // Timestamp to run callback at.
    timestamp_us_t timestamp;
    // Timer callback function.
    timer_fn_t     callback;
    // Cookie for timer callback function.
    void          *cookie;
} timertask_t;

// Time CPU-local data.
typedef struct {
    // Whether the timer is set for preemption instead of timer callback.
    bool           timer_is_preempt;
    // Next time to preempt at.
    timestamp_us_t preempt_time;
} time_cpulocal_t;



// Set the CPU's timer to a certain timestamp.
void time_set_cpu_timer(timestamp_us_t timestamp);
// Clear the CPU's timer.
void time_clear_cpu_timer();
// Generic timer init after timer-specific init.
void time_init_generic();
// Callback from timer-specific code when the CPU timer fires.
void time_cpu_timer_isr();
