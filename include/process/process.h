
// SPDX-License-Identifier: MIT

#pragma once

#include "scheduler/scheduler.h"

// A process and all of its resources.
typedef struct process_t process_t;
// Globally unique process ID.
typedef int              pid_t;

// Create a new, empty process.
process_t *proc_create();
// Delete a process and release any resources it had.
void       proc_delete(pid_t pid);
// Get a process handle by ID.
process_t *proc_get(pid_t pid);

// Load and prepare the executable for the process.
void proc_load(process_t *process, char const *executable);
// Set arguments for a process.
// If omitted, argc will be 0 and argv will be NULL.
void proc_setargs(process_t *process, int argc, char const *const *argv);
// Start a prepared process.
// Should be called after `proc_load` and the optional `proc_setargs`.
void proc_start(process_t *process);

// Create a new thread in a process.
// Returns created thread handle.
sched_thread_t *proc_create_thread(badge_err_t *ec, sched_entry_point_t entry_point, void *arg, sched_prio_t priority);
// Allocate more memory to a process.
// Returns actual virtual address on success, 0 on failure.
size_t          proc_map(process_t *map, size_t vaddr_req, size_t min_size, size_t min_align);
// Release memory allocated to a process.
void            proc_unmap(process_t *map, size_t base);
