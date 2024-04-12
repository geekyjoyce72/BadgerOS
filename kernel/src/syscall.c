
// SPDX-License-Identifier: MIT

#include "syscall.h"

#include "cpu/isr.h"
#include "cpu/isr_ctx.h"
#include "cpu/panic.h"
#include "log.h"
#include "process/internal.h"
#include "process/process.h"
#include "rawprint.h"
#include "scheduler/cpu.h"
#include "scheduler/scheduler.h"



// Called on invalid system call.
static void invalid_syscall(long sysno) {
    isr_ctx_t *ctx = isr_ctx_get();

    // Report error.
    rawprint("Invalid syscall ");
    rawprintdec(sysno, 0);
    rawprint(" at PC 0x");
    rawprinthex(ctx->regs.pc, sizeof(ctx->regs.pc) * 2);
    rawprint("\n");
    isr_ctx_dump(ctx);

    // Terminate thread.
    proc_exit_self(-1);
}

// Temporary write system call.
void syscall_temp_write(char const *message, size_t length) {
    process_t *const proc = proc_current();
    if (proc_map_contains_raw(proc, (size_t)message, length) & MEMPROTECT_FLAG_R) {
        mutex_acquire(NULL, &log_mtx, TIMESTAMP_US_MAX);
        rawprint_substr(message, length);
        mutex_release(NULL, &log_mtx);
    }
}

// Shutdown system call implementation.
extern void syscall_sys_shutdown(bool is_reboot);

// System call handler jump table thing.
SYSCALL_HANDLER_SIGNATURE {
    SYSCALL_HANDLER_IGNORE_UNUSED;

    long long retval = 0;
    switch (sysnum) {
        case SYSCALL_TEMP_WRITE: syscall_temp_write((char const *)a0, a1); break;
        case SYSCALL_THREAD_YIELD: sched_yield(); break;
        case SYSCALL_SELF_EXIT: syscall_self_exit(a0); break;
        case SYSCALL_SYS_SHUTDOWN: syscall_sys_shutdown(a0); break;
        case SYSCALL_FS_OPEN: retval = syscall_fs_open((char const *)a0, a1, a2); break;
        case SYSCALL_FS_CLOSE: retval = syscall_fs_close(a0); break;
        case SYSCALL_FS_READ: retval = syscall_fs_read(a0, (void *)a1, a2); break;
        case SYSCALL_FS_WRITE: retval = syscall_fs_write(a0, (void const *)a1, a2); break;
        case SYSCALL_FS_GETDENTS: retval = syscall_fs_getdents(a0, (void *)a1, a2); break;
        case SYSCALL_MEM_ALLOC: retval = (long long)syscall_mem_alloc(a0, a1, a2, a3); break;
        case SYSCALL_MEM_SIZE: retval = syscall_mem_size((void *)a0); break;
        case SYSCALL_MEM_DEALLOC: syscall_mem_dealloc((void *)a0); break;
        default: invalid_syscall(sysnum); break;
    }
    syscall_return(retval);
}
