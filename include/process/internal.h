
// SPDX-License-Identifier: MIT

#pragma once

#include "process/process.h"

// Kill a process from one of its own threads.
void proc_exit_self(int code);

// Look up a process without locking the global process mutex.
process_t *proc_get_unsafe(pid_t pid);

// Suspend all threads for a process except the current.
void proc_suspend(process_t *process, sched_thread_t *current);
// Resume all threads for a process.
void proc_resume(process_t *process);
// Release all process runtime resources (threads, memory, files, etc.).
// Does not remove args, exit code, etc.
void proc_delete_runtime(process_t *process);

// Create a new, empty process.
process_t *proc_create_raw(badge_err_t *ec);
// Get a process handle by ID.
process_t *proc_get(pid_t pid);
// Get the process' flags.
uint32_t   proc_getflags_raw(process_t *process);

// Set arguments for a process.
// If omitted, argc will be 0 and argv will be NULL.
void proc_setargs_raw(badge_err_t *ec, process_t *process, int argc, char const *const *argv);
// Load an executable and start a prepared process.
void proc_start_raw(badge_err_t *ec, process_t *process, char const *executable);

// Create a new thread in a process.
// Returns created thread handle.
sched_thread_t *proc_create_thread_raw_unsafe(
    badge_err_t *ec, process_t *process, sched_entry_point_t entry_point, void *arg, sched_prio_t priority
);
// Delete a thread in a process.
void   proc_delete_thread_raw_unsafe(badge_err_t *ec, process_t *process, sched_thread_t *thread);
// Allocate more memory to a process.
// Returns actual virtual address on success, 0 on failure.
size_t proc_map_raw(badge_err_t *ec, process_t *process, size_t vaddr_req, size_t min_size, size_t min_align);
// Release memory allocated to a process.
void   proc_unmap_raw(badge_err_t *ec, process_t *process, size_t base);
