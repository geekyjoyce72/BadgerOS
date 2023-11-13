
// SPDX-License-Identifier: MIT

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>



// Maximum number of mapped regions per process.
#define PROC_MEMMAP_MAX_REGIONS 8

// Process memory map information.
typedef struct proc_memmap_t {
    // Base address of code segments.
    size_t segs_base;
    // Number of mapped regions.
    size_t regions_len;
    // Base addresses of memory mapped regions.
    size_t region_bases[PROC_MEMMAP_MAX_REGIONS];
    // Sizes of memory mapped regions.
    size_t region_sizes[PROC_MEMMAP_MAX_REGIONS];
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
