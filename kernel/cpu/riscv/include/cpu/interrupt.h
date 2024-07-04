
// SPDX-License-Identifier: MIT

#pragma once

#include "cpu/regs.h"

#include <stdbool.h>

// Globally enable/disable interrupts in this CPU.
// Returns whether interrupts were enabled.
static inline bool irq_enable(bool enable) {
    long mask = CSR_STATUS_PP_MASK << CSR_STATUS_PP_BASE_BIT;
    if (enable) {
        asm volatile("csrrs %0, " CSR_STATUS_STR ", %0" : "+r"(mask));
    } else {
        asm volatile("csrrc %0, " CSR_STATUS_STR ", %0" : "+r"(mask));
    }
    return (mask >> CSR_STATUS_PP_BASE_BIT) & 1;
}

// Query whether interrupts are enabled in this CPU.
static inline bool irq_enabled() {
    long mask;
    asm("csrr %0, " CSR_STATUS_STR : "=r"(mask));
    return (mask >> CSR_STATUS_PP_BASE_BIT) & 1;
}
