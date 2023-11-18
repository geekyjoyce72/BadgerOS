
// SPDX-License-Identifier: MIT

#pragma once

#include "cpu/process/types.h"
#include "port/hardware_allocation.h"
#include "port/process/types.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>



// A memory map entry.
typedef struct {
    // Base address of the region.
    size_t base;
    // Size of the region.
    size_t size;
    // Write permission.
    bool   write;
    // Execution permission.
    bool   exec;
} proc_memmap_ent_t;

// Process memory map information.
typedef struct proc_memmap_t {
    // Memory management cache.
    proc_mpu_t        mpu;
    // Base address of code segments.
    size_t            segs_base;
    // Number of mapped regions.
    size_t            regions_len;
    // Mapped regions.
    proc_memmap_ent_t regions[PROC_MEMMAP_MAX_REGIONS];
} proc_memmap_t;

// Process ID.
typedef int pid_t;

// Userland process.
typedef struct process_t {
    // Process ID.
    pid_t         pid;
    // Memory map information.
    proc_memmap_t memmap;
} process_t;
