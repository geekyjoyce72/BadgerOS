
// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <kernel_ctx.h>

// Type used for storing registers.
typedef riscv_regs_t isr_regs_t;

// Install interrupt and trap handlers.
// Requires a preallocated context and regs struct.
void interrupt_init(kernel_ctx_t *ctx, isr_regs_t *regs);
