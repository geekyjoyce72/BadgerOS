
# SPDX-License-Identifier: MIT

#include "port/hardware.h"



    # Run a restricted function and catch exceptions.
    .section ".text.isr_noexc_run", "ax", @progbits
    .global isr_noexc_run_int
    .type isr_noexc_run_int, %function
isr_noexc_run_int:
    ret
