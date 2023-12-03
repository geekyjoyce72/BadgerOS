
// SPDX-License-Identifier: MIT

#include "badge_err.h"
#include "filesystem.h"
#include "housekeeping.h"
#include "log.h"
#include "malloc.h"
#include "memprotect.h"
#include "port/interrupt.h"
#include "process/process.h"
#include "scheduler/scheduler.h"
#include "time.h"

#include <stdatomic.h>

/*
// Temporary kernel context until threading is implemented.
static isr_ctx_t kctx;

void debug_func(void *);
#define stack_size 8192
static uint8_t stack0[stack_size] ALIGNED_TO(STACK_ALIGNMENT);

static inline void check_ec(badge_err_t *ec) {
    if (ec && !badge_err_is_ok(ec)) {
        logkf(LOG_ERROR, "ELOC=%{d}, ECAUSE=%{d}", ec->location, ec->cause);
        sched_destroy_thread(NULL, sched_get_current_thread());
    }
}

// This is the entrypoint after the stack has been set up and the init functions
// have been run. Main is not allowed to return, so declare it noreturn.
void main() NORETURN;
void main() {
    badge_err_t err;

    // Install interrupt and trap handlers.
    interrupt_init(&kctx);

    // Set up memory protection.
    memprotect_init();

    // Set up timers and watchdogs.
    // This function must run within the first ~1s of power-on time and should be
    // called as early as possible.
    time_init();

    // Set up memory allocator.
    kernel_heap_init();

    // Set up multithreading.
    sched_init(&err);
    badge_err_assert_always(&err);
    hk_init();

    // A debug thread.
    sched_thread_t *const debug_thread_0 =
        sched_create_kernel_thread(&err, debug_func, NULL, stack0, stack_size, SCHED_PRIO_NORMAL);
    badge_err_assert_always(&err);
    sched_set_name(&err, debug_thread_0, "debug0");
    badge_err_assert_always(&err);
    sched_resume_thread(&err, debug_thread_0);
    badge_err_assert_always(&err);

    // Enter the scheduler
    sched_exec();
    __builtin_unreachable();
}

void debug_func(void *arg) {
    (void)arg;
    badge_err_t ec;

    // Create RAMFS.
    logk(LOG_DEBUG, "Creating RAMFS at /");
    fs_mount(&ec, FS_TYPE_RAMFS, NULL, "/", 0);
    check_ec(&ec);

    // Put the ROM in the RAMFS.
    file_t fd = fs_open(&ec, "/a.out", OFLAGS_CREATE | OFLAGS_WRITEONLY);
    check_ec(&ec);
    fs_write(&ec, fd, elf_rom, (long)elf_rom_len);
    check_ec(&ec);
    fs_close(&ec, fd);
    check_ec(&ec);

    // Start a process.
    pid_t pid = proc_create(&ec);
    check_ec(&ec);
    proc_start(&ec, pid, "/a.out");
    check_ec(&ec);
}
*/



// The initial kernel stack.
extern char          stack_bottom[] asm("__stack_bottom");
extern char          stack_top[] asm("__stack_top");
// When set, a shutdown is initiated.
// 0: Do nothing.
// 1: Shut down (default).
// 2: Reboot.
atomic_int           kernel_shutdown_mode;
// Temporary file image.
extern uint8_t const elf_rom[];
extern size_t const  elf_rom_len;


#define show_csr(name)                                                                                                 \
    do {                                                                                                               \
        long csr;                                                                                                      \
        asm("csrr %0, " #name : "=r"(csr));                                                                            \
        logkf(LOG_INFO, #name ": %{long;x}", csr);                                                                     \
    } while (0)

static void kernel_init();
static void userland_init();
// static void userland_shutdown();
// static void kernel_shutdown();

// Manages the kernel's lifetime after basic runtime initialization.
static void kernel_lifetime_func() {
    // Start the kernel services.
    kernel_init();
    // TODO: Start other CPUs.
    // cpu_multicore_init();
    // Start userland.
    userland_init();



    // The boot process is now complete, this thread will wait until a shutdown is issued.
    int shutdown_mode;
    do {
        sched_yield();
        shutdown_mode = atomic_load(&kernel_shutdown_mode);
    } while (shutdown_mode == 0);

    // TODO: Shutdown process.
    logk(LOG_INFO, "TODO: Shutdown procedure.");
    while (1) continue;
}

// Shutdown system call implementation.
void syscall_sys_shutdown(bool is_reboot) {
    logk(LOG_INFO, is_reboot ? "Reboot requested" : "Shutdown requested");
    atomic_store(&kernel_shutdown_mode, 1 + is_reboot);
}



// After control handover, the booting CPU core starts here and other cores wait.
// This sets up the basics of everything needed by the other systems of the kernel.
// When finished, the booting CPU will perform kernel initialization.
void basic_runtime_init() {
    badge_err_t ec = {0};
    isr_ctx_t   tmp_ctx;

    // ISR initialization.
    interrupt_init(&tmp_ctx);
    // Early platform initialization.
    // port_early_init();

    // Timekeeping initialization.
    time_init();

    // Page allocator initialization.
    // page_alloc_init();
    // Kernel memory allocator initialization.
    kernel_heap_init();
    // Memory protection initialization.
    memprotect_init();

    // Scheduler initialization.
    sched_init();
    // Housekeeping thread initialization.
    hk_init();
    // Add the remainder of the kernel lifetime as a new thread.
    sched_thread_t *thread = sched_create_kernel_thread(
        &ec,
        kernel_lifetime_func,
        NULL,
        stack_bottom,
        stack_top - stack_bottom,
        SCHED_PRIO_NORMAL
    );
    badge_err_assert_always(&ec);
    sched_resume_thread(&ec, thread);
    badge_err_assert_always(&ec);

    // Full hardware initialization.
    // port_init();

    // Start the scheduler and enter the next phase in the kernel's lifetime.
    sched_exec();
}



// After basic runtime initialization, the booting CPU core continues here.
// This finishes the initialization of all kernel systems, resources and services.
// When finished, the non-booting CPUs will be started (method and entrypoints to be determined).
static void kernel_init() {
    badge_err_t ec = {0};
    logk(LOG_INFO, "BadgerOS starting...");

    // Temporary filesystem image.
    fs_mount(&ec, FS_TYPE_RAMFS, NULL, "/", 0);
    badge_err_assert_always(&ec);

    fs_dir_create(&ec, "/sbin");
    badge_err_assert_always(&ec);

    file_t fd = fs_open(&ec, "/sbin/init", OFLAGS_CREATE | OFLAGS_WRITEONLY);
    badge_err_assert_always(&ec);
    fs_write(&ec, fd, elf_rom, (long)elf_rom_len);
    badge_err_assert_always(&ec);
    fs_close(&ec, fd);
    badge_err_assert_always(&ec);
}



// After kernel initialization, the booting CPU core continues here.
// This starts up the `init` process while other CPU cores wait for processes to be scheduled for them.
// When finished, this function returns and the thread should wait for a shutdown event.
static void userland_init() {
    badge_err_t ec = {0};
    logk(LOG_INFO, "Kernel initialized");
    logk(LOG_INFO, "Staring init process");

    pid_t pid = proc_create(&ec);
    badge_err_assert_always(&ec);
    assert_dev_drop(pid == 1);
    proc_start(&ec, pid, "/sbin/init");
    badge_err_assert_always(&ec);
}



// // When a shutdown event begins, exactly one CPU core runs this entire function.
// // This signals all processes to exit (or be killed if they wait too long) and shuts down other CPU cores.
// // When finished, the CPU continues to shut down the kernel.
// static void userland_shutdown() {
// }



// // When the userspace has been shut down, the CPU continues here.
// // This will synchronize all filesystems and clean up any other resources not needed to finish hardware shutdown.
// // When finished, the CPU continues to the platform-specific hardware shutdown / reboot handler.
// static void kernel_shutdown() {
// }
