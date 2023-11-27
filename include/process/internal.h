
// SPDX-License-Identifier: MIT

#pragma once

#include "process/process.h"

// Look up a process without locking the global process mutex.
process_t *proc_get_unsafe(pid_t pid);

// Suspend all threads for a process except the current.
void proc_suspend(process_t *process, sched_thread_t *current);
// Resume all threads for a process.
void proc_resume(process_t *process);
// Release all process runtime resources (threads, memory, files, etc.).
// Does not remove args, exit code, etc.
void proc_delete_runtime(process_t *process);
// Thread-safe process clean-up.
void proc_cleanup(pid_t pid);
