
// SPDX-License-Identifier: MIT

#pragma once

#include "port/process/process.h"
#include "process/types.h"

extern process_t dummy_proc;

static inline process_t *proc_get(pid_t pid) {
    (void)pid;
    return &dummy_proc;
}

// Allocate more memory to a process.
// Returns actual virtual address on success, 0 on failure.
size_t proc_map(process_t *map, size_t vaddr_req, size_t min_size, size_t min_align);
// Release memory allocated to a process.
void   proc_unmap(process_t *map, size_t base);
