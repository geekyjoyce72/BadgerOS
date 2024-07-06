
// SPDX-License-Identifier: MIT

#include "page_alloc.h"

#include "badge_strings.h"
#include "port/hardware_allocation.h"
#include "static-buddy.h"
#if MEMMAP_VMEM
#include "cpu/mmu.h"
#endif



// Allocate pages of physical memory.
// Uses physical page numbers (paddr / MEMMAP_PAGE_SIZE).
size_t phys_page_alloc(size_t page_count, bool for_user) {
    void *mem = buddy_allocate(page_count * MEMMAP_PAGE_SIZE, for_user ? BLOCK_TYPE_USER : BLOCK_TYPE_PAGE, 0);
    if (!mem) {
        return 0;
    }
    mem_set(mem, 0, page_count * MEMMAP_PAGE_SIZE);
#if MEMMAP_VMEM
    return ((size_t)mem - mmu_hhdm_vaddr) / MEMMAP_PAGE_SIZE;
#else
    return (size_t)mem / MEMMAP_PAGE_SIZE;
#endif
}

// Free pages of physical memory.
// Uses physical page numbers (paddr / MEMMAP_PAGE_SIZE).
void phys_page_free(size_t ppn) {
#if MEMMAP_VMEM
    buddy_deallocate((void *)((ppn + mmu_hhdm_vpn) * MEMMAP_PAGE_SIZE));
#else
    buddy_deallocate((void *)(ppn * MEMMAP_PAGE_SIZE));
#endif
}
