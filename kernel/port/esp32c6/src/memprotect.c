
// SPDX-License-Identifier: MIT

#include "memprotect.h"

#include "assertions.h"
#include "cpu/riscv_pmp.h"
#include "isr_ctx.h"
#include "port/hardware_allocation.h"
#include "port/interrupt.h"

// Initialise memory protection driver.
void memprotect_early_init() {
    // Initialise PMP driver.
    riscv_pmp_init();

    // Add lower NULLPTR protection range.
    riscv_pmpaddr_write(PMP_ENTRY_NULLPTR_LOW_NAPOT, riscv_pmpaddr_calc_napot(0, PMP_SIZE_NULLPTR));
    riscv_pmpcfg_set(
        PMP_ENTRY_NULLPTR_LOW_NAPOT,
        ((riscv_pmpcfg_t){
            .read            = false,
            .write           = false,
            .exec            = false,
            .addr_match_mode = RISCV_PMPCFG_NAPOT,
            .lock            = true,
        })
    );

    // Add upper NULLPTR protection range.
    riscv_pmpaddr_write(PMP_ENTRY_NULLPTR_HIGH_NAPOT, riscv_pmpaddr_calc_napot(-PMP_SIZE_NULLPTR, PMP_SIZE_NULLPTR));
    riscv_pmpcfg_set(
        PMP_ENTRY_NULLPTR_HIGH_NAPOT,
        ((riscv_pmpcfg_t){
            .read            = false,
            .write           = false,
            .exec            = false,
            .addr_match_mode = RISCV_PMPCFG_NAPOT,
            .lock            = true,
        })
    );

    // Add ROM write-protect range.
    riscv_pmpaddr_write(PMP_ENTRY_ROM_WP_NAPOT, riscv_pmpaddr_calc_napot(PMP_BASE_ROM_WP, PMP_SIZE_ROM_WP));
    riscv_pmpcfg_set(
        PMP_ENTRY_ROM_WP_NAPOT,
        ((riscv_pmpcfg_t){
            .read            = true,
            .write           = false,
            .exec            = true,
            .addr_match_mode = RISCV_PMPCFG_NAPOT,
            .lock            = true,
        })
    );

    // Add FLASH write-protect range.
    riscv_pmpaddr_write(PMP_ENTRY_FLASH_WP_NAPOT, riscv_pmpaddr_calc_napot(PMP_BASE_FLASH_WP, PMP_SIZE_FLASH_WP));
    riscv_pmpcfg_set(
        PMP_ENTRY_FLASH_WP_NAPOT,
        ((riscv_pmpcfg_t){
            .read            = true,
            .write           = false,
            .exec            = true,
            .addr_match_mode = RISCV_PMPCFG_NAPOT,
            .lock            = true,
        })
    );

    // Add user global permissions.
    // TODO: These permissions will be NONE later, but are ALL for now.
    riscv_pmpaddr_write(PMP_ENTRY_USER_GLOBAL_NAPOT, RISCV_PMPADDR_NAPOT_GLOBAL);
    riscv_pmpcfg_set(
        PMP_ENTRY_USER_GLOBAL_NAPOT,
        ((riscv_pmpcfg_t){
            .read            = false,
            .write           = false,
            .exec            = false,
            .addr_match_mode = RISCV_PMPCFG_NAPOT,
            .lock            = false,
        })
    );
}

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

// Add a memory protection region for user memory.
bool memprotect_u(proc_memmap_t *new_mm, mpu_ctx_t *ctx, size_t vaddr, size_t paddr, size_t length, uint32_t flags) {
    return vaddr == paddr && riscv_pmp_memprotect(new_mm, ctx, vaddr, length, flags);
}

// Add a memory protection region for kernel memory.
bool memprotect_k(size_t vaddr, size_t paddr, size_t length, uint32_t flags) {
    (void)length;
    (void)flags;
    return vaddr == paddr;
}

// Commit pending memory protections, if any.
void memprotect_commit(mpu_ctx_t *ctx) {
    (void)ctx;
}
