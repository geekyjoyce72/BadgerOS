
// SPDX-License-Identifier: MIT

#include "memprotect.h"

#include "assertions.h"
#include "cpu/riscv_pmp.h"
#include "isr_ctx.h"
#include "port/hardware_allocation.h"
#include "port/interrupt.h"

// Initialise memory protection driver.
void memprotect_init() {
}



// Create a memory protection context.
void memprotect_create(mpu_ctx_t *ctx) {
    (void)ctx;
}

// Clean up a memory protection context.
void memprotect_destroy(mpu_ctx_t *ctx) {
    (void)ctx;
}

// Add a memory protection region.
bool memprotect(proc_memmap_t *new_mm, mpu_ctx_t *ctx, size_t vaddr, size_t paddr, size_t length, uint32_t flags) {
    (void)new_mm;
    (void)ctx;
    (void)length;
    (void)flags;
    return (vaddr | paddr) % MEMMAP_PAGE_SIZE == 0;
}

// Commit pending memory protections, if any.
void memprotect_commit(mpu_ctx_t *ctx) {
    (void)ctx;
}

// Swap in memory protections for the given context.
void memprotect_swap_from_isr() {
}
