
// SPDX-License-Identifier: MIT

#include "process/process.h"

#include "cpu/process/process.h"
#include "port/hardware_allocation.h"



// Generate RISC-V PMP configurations cache for a given process.
// Returns `true` if the memory map can be represented with the configured amount of PMPs.
bool proc_riscv_pmp_gen(proc_memmap_t *memmap) {
    // TODO.
    (void)memmap;
    return true;
}

// Swap in RISC-V PMP configurations for a given process.
void proc_riscv_pmp_swap(proc_riscv_pmp_t const *pmp) {
    // TODO.
    (void)pmp;
}
