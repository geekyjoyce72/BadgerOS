
// SPDX-License-Identifier: MIT

#include "time.h"

#include "arrays.h"
#include "assertions.h"
#include "config.h"
#include "hwtimer.h"
#include "interrupt.h"
#include "mutex.h"
#include "scheduler/isr.h"
#include "smp.h"
#include "time_private.h"

#ifdef CONFIG_TARGET_esp32p4
// Timer used for task list items.
#define TIMER_COUNT 2
#else
// Timer used for task list items.
#define TIMER_COUNT 1
#endif



// Callback to the timer driver for when a timer alarm fires.
void timer_isr_timer_alarm() {
    int cpu = smp_cur_cpu();
    timer_alarm_disable(cpu);
    timer_int_clear(cpu);
    time_cpu_timer_isr();
}



// Set the CPU's timer to a certain timestamp.
void time_set_cpu_timer(timestamp_us_t timestamp) {
    timer_alarm_config(smp_cur_cpu(), timestamp, false);
}

// Clear the CPU's timer.
void time_clear_cpu_timer() {
    int cpu = smp_cur_cpu();
    timer_alarm_disable(cpu);
    timer_int_clear(cpu);
}



// Initialise timer and watchdog subsystem.
void time_init() {
    timer_init();

    // Configure system timers.
    for (int i = 0; i <= TIMER_COUNT; i++) {
        timer_stop(i);
        timer_value_set(i, 0);
        timer_alarm_disable(i);
        timer_int_clear(i);
        timer_int_enable(i, true);
        timer_set_freq(i, 1000000);
    }

    // Assign timer IRQs.
    for (int i = 0; i < TIMER_COUNT; i++) {
        set_cpu_timer_irq(i, timer_get_irq(i));
    }

    // Start timers at close to the same time.
    for (int i = 0; i <= TIMER_COUNT; i++) {
        timer_start(i);
    }

    time_init_generic();
}

// Get current time in microseconds.
timestamp_us_t time_us() {
    return timer_value_get(smp_cur_cpu());
}
