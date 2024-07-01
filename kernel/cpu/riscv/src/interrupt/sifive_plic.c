
// SPDX-License-Identifier: MIT

#include "interrupt.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"



// Route an external interrupt to an internal interrupt on this CPU.
void irq_ch_route(int ext_irq, int int_irq) {
}
// Query whether an external interrupt is pending.
bool irq_ch_ext_pending(int ext_irq) {
    return false;
}
