
// SPDX-License-Identifier: MIT

#include "cpu/mmu.h"

#include "assertions.h"
#include "cpu/panic.h"
#include "isr_ctx.h"
#include "limine.h"
#include "log.h"
#include "memprotect.h"
#include "page_alloc.h"
#include "port/hardware_allocation.h"

_Static_assert(MEMMAP_PAGE_SIZE == MMU_PAGE_SIZE);

// Virtual address offset of the higher half.
size_t mmu_high_vaddr;
// Size of a "half".
size_t mmu_half_size;
// Number of page table levels.
int    mmu_levels;

// MPU context for kernel threads not associated with a user thread.
static mpu_ctx_t k_mpu_ctx;

extern char const __start_badgeros_kernel[];
extern char const __stop_badgeros_kernel[];



// Load a word from physical memory.
static inline size_t pmem_load(size_t paddr) {
    assert_dev_drop(paddr < mmu_half_size / 2);
    return *(size_t volatile *)(paddr + mmu_high_vaddr);
}

// Store a word to physical memory.
static inline size_t pmem_store(size_t paddr, size_t data) {
    assert_dev_drop(paddr < mmu_half_size / 2);
    *(size_t volatile *)(paddr + mmu_high_vaddr) = data;
}

// Get the index from the VPN for a given page table level.
static size_t get_vpn_i(size_t vpn, int pt_level) {
    return (vpn >> (9 * pt_level)) & 0x1ff;
}

// Walk a page table; vaddr to paddr or return where the page table ends.
mmu_walk_t mmu_walk(size_t pt_ppn, size_t vpn) {
    size_t pt_addr = pt_ppn << 12;
    size_t vaddr   = vpn << 12;
    if (vaddr >= mmu_half_size && vaddr < mmu_high_vaddr) {
        return (mmu_walk_t){0};
    }

    // Place the next VPN at the MSB for easy extraction.
    size_t    vaddr_rem = vaddr << (7 + 9 * (5 - mmu_levels));
    size_t    pte_addr  = 0;
    mmu_pte_t pte       = {0};

    // Walk the page table.
    for (int i = 0; i < mmu_levels; i++) {
        int level = mmu_levels - i - 1;
        pte_addr  = pt_addr | (vaddr_rem >> 55 << 3);
        pte.val   = pmem_load(pte_addr);

        if (!pte.v) {
            // PTE invalid.
            return (mmu_walk_t){pte_addr, pte, level, false, true};
        } else if (pte.r || pte.w || pte.x) {
            // Leaf PTE.
            if (level && pte.ppn & ((1 << 9 * level) - 1)) {
                // Misaligned superpage.
                logkf(LOG_FATAL, "PT corrupt: L%{d} leaf PTE @ %{size;x} (%{size;x}) misaligned", i, pte_addr, pte.val);
                logkf(LOG_FATAL, "Offending VADDR: %{size;x}", vaddr);
                panic_abort();
            }
            // Valid leaf PTE.
            return (mmu_walk_t){pte_addr, pte, level, true, true};
        } else if (level) {
            // Non-leaf PTE.
            pt_addr     = pte.ppn << 12;
            vaddr_rem <<= 9;
        }
    }

    // Not a leaf node.
    logkf(LOG_FATAL, "PT corrupt: L0 PTE @ %{size;x} (%{size;x}) not a leaf PTE", pte_addr, pte.val);
    logkf(LOG_FATAL, "Offending VADDR: %{size;x}", vaddr);
    panic_abort();
}

// Garbage-collect unused page table pages.
void mmu_gc(size_t pt_ppn) {
    // TODO.
    (void)pt_ppn;
}

// Map a single page.
bool mmu_map_1(size_t pt_ppn, size_t vpn, size_t ppn, uint32_t flags) {
    mmu_walk_t walk = mmu_walk(pt_ppn, vpn);
    for (int i = walk.level; i > 0; i++) {
        size_t part_ppn = phys_page_alloc(1, false);
        if (!part_ppn) {
            return false;
        }
        size_t pte_addr = (walk.pte.ppn << 12) | (get_vpn_i(vpn, i) << 3);

        mmu_pte_t pte = {
            .v   = true,
            .ppn = part_ppn,
        };
        pmem_store(pte_addr, pte.val);

        walk.level   = i;
        walk.pte.val = 0;
        walk.paddr   = (part_ppn << 12) | (get_vpn_i(vpn, i - 1) << 3);
    }
    walk.pte.v   = 1;
    walk.pte.r   = !(flags & MEMPROTECT_FLAG_R);
    walk.pte.w   = !(flags & MEMPROTECT_FLAG_W);
    walk.pte.x   = !(flags & MEMPROTECT_FLAG_X);
    walk.pte.u   = !(flags & MEMPROTECT_FLAG_KERNEL);
    walk.pte.g   = !(flags & MEMPROTECT_FLAG_GLOBAL);
    walk.pte.ppn = ppn;
    pmem_store(walk.paddr, walk.pte.val);
}

// Unmap a single page.
void mmu_unmap_1(size_t pt_ppn, size_t vpn) {
    mmu_walk_t walk = mmu_walk(pt_ppn, vpn);
    if (walk.found) {
        pmem_store(walk.paddr, 0);
    }
}



// MMU-specific init code.
void mmu_init() {
    // Read paging mode from SATP.
    riscv_satp_t satp = get_satp();
    mmu_levels        = satp.mode - RISCV_SATP_SV39 + 3;
    mmu_half_size     = 1LLU << (11 + 9 * mmu_levels);
    mmu_high_vaddr    = -mmu_half_size;
    logkf_from_isr(LOG_DEBUG, "Paging levels:    %{d}", mmu_levels);
    logkf_from_isr(LOG_DEBUG, "Higher half addr: %{size;x}", mmu_high_vaddr);
}



// Create a memory protection context.
void memprotect_create(mpu_ctx_t *ctx) {
    (void)ctx;
}

// Clean up a memory protection context.
void memprotect_destroy(mpu_ctx_t *ctx) {
    (void)ctx;
}

// Manage a memory protection region for user memory.
bool memprotect_u(proc_memmap_t *new_mm, mpu_ctx_t *ctx, size_t vaddr, size_t paddr, size_t length, uint32_t flags) {
    (void)new_mm;
    if ((vaddr | paddr | length) % MEMMAP_PAGE_SIZE) {
        return false;
    }
    return true;
}

// Manage a memory protection region for kernel memory.
bool memprotect_k(size_t vaddr, size_t paddr, size_t length, uint32_t flags) {
    if ((vaddr | paddr | length) % MEMMAP_PAGE_SIZE) {
        return false;
    }
    return true;
}

// Commit pending memory protections, if any.
void memprotect_commit(mpu_ctx_t *ctx) {
    (void)ctx;
    asm("sfence.vma");
}

// Swap in memory protections for the given context.
void memprotect_swap_from_isr() {
    mpu_ctx_t *mpu = isr_ctx_get()->mpu_ctx;
    if (mpu) {
        riscv_satp_t satp = {
            .ppn  = mpu->root_ppn,
            .asid = 0,
            .mode = RISCV_SATP_SV39,
        };
        asm("csrw satp, %0; sfence.vma" ::"r"(satp));
    }
}
