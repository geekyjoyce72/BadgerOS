
// SPDX-License-Identifier: MIT

#pragma once

#include "memprotect.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#define MMU_PAGE_SIZE     0x1000LLU
#define MMU_MEGAPAGE_SIZE (MMU_PAGE_SIZE << 9)
#define MMU_GIGAPAGE_SIZE (MMU_PAGE_SIZE << 18)
#define MMU_TERAPAGE_SIZE (MMU_PAGE_SIZE << 27)
#define MMU_PETAPAGE_SIZE (MMU_PAGE_SIZE << 36)

// PBMT modes.
typedef enum {
    // Use values from applicable PMA(s).
    RISCV_PBMT_PMA,
    // Non-cacheable idempotent main memory.
    RISCV_PBMT_NC,
    // Non-cacheable strongly-ordered I/O memory.
    RISCV_PBMT_IO,
} riscv_pbmt_t;

// Address translation modes.
typedef enum {
    // Direct-mapped; virtual memory disabled.
    RISCV_SATP_BARE = 8,
    // RISC-V page-based 39-bit virtual memory.
    RISCV_SATP_SV39 = 8,
    // RISC-V page-based 48-bit virtual memory.
    RISCV_SATP_SV48,
    // RISC-V page-based 57-bit virtual memory.
    RISCV_SATP_SV57,
} riscv_satp_mode_t;

// RISC-V MMU page table entry.
typedef union {
    struct {
        // Valid entry; entry is ignored when 0.
        size_t v    : 1;
        // Allow reading.
        size_t r    : 1;
        // Allow writing; requires reading.
        size_t w    : 1;
        // Allow executing.
        size_t x    : 1;
        // Page belongs to U-mode.
        size_t u    : 1;
        // Global page present in all page tables.
        size_t g    : 1;
        // Page has been accessed before.
        size_t a    : 1;
        // Page has been written to.
        size_t d    : 1;
        // Reserved for supervisor software.
        size_t rsw  : 2;
        // Physical page number.
        size_t ppn  : 44;
        // Reserved.
        size_t      : 7;
        // PBMT mode.
        size_t pbmt : 2;
        // NAPOT page table entry.
        size_t n    : 1;
    };
    size_t val;
} mmu_pte_t;

// RISC-V SATP CSR format.
typedef union {
    struct {
        // Page table root physical page number.
        size_t ppn  : 44;
        // Address-space ID.
        size_t asid : 16;
        // Address translation mode.
        size_t mode : 4;
    };
    size_t val;
} riscv_satp_t;

// Page table walk result.
typedef struct {
    // Physical address of last loaded PTA.
    size_t    paddr;
    // Last loaded PTE.
    mmu_pte_t pte;
    // Page table level of PTE.
    uint8_t   level;
    // Whether the subject page was found.
    bool      found;
    // Whether the virtual address is valid.
    bool      vaddr_valid;
} mmu_walk_t;



// Virtual address offset of the higher half.
extern size_t mmu_high_vaddr;
// Size of a "half".
extern size_t mmu_half_size;
// Number of page table levels.
extern int    mmu_levels;
// Virtual page number of the higher half.
#define mmu_high_vpn   (mmu_high_vaddr / MMU_PAGE_SIZE)
// Virtual page size of a "half".
#define mmu_half_pages (mmu_half_size / MMU_PAGE_SIZE)



// MMU-specific init code.
void       mmu_init();
// Garbage-collect unused page table pages.
void       mmu_gc(size_t pt_ppn);
// Walk a page table; vaddr to paddr or return where the page table ends.
mmu_walk_t mmu_walk(size_t pt_ppn, size_t vpn, size_t ppn);
// Map a single page.
bool       mmu_map_1(size_t pt_ppn, size_t vpn, uint32_t flags);
// Unmap a single page.
void       mmu_unmap_1(size_t pt_ppn, size_t vpn);

static inline void set_satp(riscv_satp_t value) {
    asm("csrw satp, %0" ::"r"(value));
}

static inline riscv_satp_t get_satp() {
    riscv_satp_t res;
    asm("csrr %0, satp" : "=r"(res));
    return res;
}

static inline void mmu_vmem_fence() {
    asm("sfence.vma" ::: "memory");
}
