
// SPDX-License-Identifier: MIT

#pragma once

#include "cpu/process/process.h"
#include "process/types.h"

// Generate MPU configuration cache for a given process.
// The ESP32-C6 only has support for PMPs.
// Returns `true` if the memory map can be represented with the configured amount of PMPs.
bool proc_mpu_gen(proc_memmap_t *memmap) {
    proc_riscv_pmp_gen(memmap);
}

// Swap in MPU configurations for a given process.
// The ESP32-C6 only has support for PMPs.
void proc_riscv_pmp_swap(proc_memmap_t *memmap) {
    proc_riscv_pmp_swap(&memmap->cpu);
}
