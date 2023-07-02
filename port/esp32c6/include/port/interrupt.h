
// SPDX-License-Identifier: MIT

#pragma once

#include "cpu/kernel_ctx.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Interrupt channel used for timer alarms.
#define INT_TIMER_ALARM_CH		16
// Interrupt channel used for watchdog alarms.
#define INT_WATCHDOG_ALARM_CH	17

// Install interrupt and trap handlers.
// Requires a preallocated context and regs struct.
void interrupt_init(kernel_ctx_t *ctx);

// Disable interrupts and return whether they were enabled.
static inline bool interrupt_disable() {
	uint32_t mstatus;
	asm volatile("csrrc %0, mstatus, %1" : "=r" (mstatus) : "r" (8));
	return mstatus & 8;
}
// Enable interrupts.
static inline void interrupt_enable() {
	asm volatile("csrs mstatus, %0" :: "r" (8));
}
