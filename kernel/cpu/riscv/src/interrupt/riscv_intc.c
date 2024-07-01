
// SPDX-License-Identifier: MIT

#include "interrupt.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"



// Set the priority of an internal interrupt on this CPU, if possible.
// 0 is least priority, 255 is most priority on this CPU.
void irq_ch_prio(int int_irq, int prio) {
}
// Acknowledge an interrupt on this CPU.
void irq_ch_ack(int int_irq) {
}

// Enable/disable an internal interrupt on this CPU.
void irq_ch_enable(int int_irq, bool enable) {
}
// Query whether an internal interrupt is enabled on this CPU.
bool irq_ch_enabled(int int_irq) {
    return false;
}
// Query whether an internal interrupt is pending.
bool irq_ch_pending(int int_irq) {
    return false;
}

// Set the interrupt service routine for an interrupt on this CPU.
void irq_ch_set_isr(int int_irq, isr_t isr) {
}

// PLIC interrupt handler.
void riscv_interrupt_handler() {
}
