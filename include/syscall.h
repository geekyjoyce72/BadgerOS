
// SPDX-License-Identifier: MIT

#pragma once



// System call definitions.
#ifdef __ASSEMBLER__
#define SYSCALL_DEF(num, name) .equ name, num
#else
#define SYSCALL_DEF(num, name) name = num,
typedef enum {
#endif



/* ==== TEMPORARY SYSCALLS ==== */
// TEMPORARY: Write one or more characters to the log.
// void(char const *message, size_t length)
SYSCALL_DEF(0xff00, SYSCALL_TEMP_WRITE)



/* ==== THREAD SYSCALLS ==== */
// Yield to other threads.
// void(void)
SYSCALL_DEF(0x0100, SYSCALL_THREAD_YIELD)

// Create a new thread.
// bool(void *entry, void *arg, int priority)
SYSCALL_DEF(0x0101, SYSCALL_THREAD_CREATE)

// Suspend a thread; it will not run again until resumed.
// bool(tid_t thread)
SYSCALL_DEF(0x0102, SYSCALL_THREAD_SUSPEND)

// Resume a thread; this does nothing if it is already running.
// bool(tid_t thread)
SYSCALL_DEF(0x0103, SYSCALL_THREAD_RESUME)

// Detach a thread; the thread will be destroyed as soon as it exits.
// bool(tid_t thread)
SYSCALL_DEF(0x0104, SYSCALL_THREAD_DETACH)

// Destroy a thread; it will be stopped and its resources released.
// bool(tid_t thread)
SYSCALL_DEF(0x0105, SYSCALL_THREAD_DESTROY)

// Exit the current thread; exit code can be read unless destroyed.
// void(int code) NORETURN
SYSCALL_DEF(0x0106, SYSCALL_THREAD_EXIT)



/* ==== SELF MANAGEMENT SYSCALLS ==== */
// Exit the process; exit code can be read by parent process.
// void(int code) NORETURN
SYSCALL_DEF(0x0200, SYSCALL_SELF_EXIT)



/* ==== FILESYSTEM SYSCALLS ==== */
// Open a file, optionally relative to a directory.
// Returns <= -1 on error, file descriptor number of success.
// file_t(char const *path, file_t relative_to, int oflags)
SYSCALL_DEF(0x0300, SYSCALL_FS_OPEN)

// Flush and close a file.
// bool(file_t fd)
SYSCALL_DEF(0x0301, SYSCALL_FS_CLOSE)

// Read bytes from a file.
// Returns -1 on EOF, <= -2 on error, read count on success.
// long(file_t fd, void *read_buf, long read_len)
SYSCALL_DEF(0x0302, SYSCALL_FS_READ)

// Write bytes to a file.
// Returns <= -1 on error, write count on success.
// long(file_t fd, void const *write_buf, long write_len)
SYSCALL_DEF(0x0303, SYSCALL_FS_WRITE)

// Read directory entries from a directory handle.
// See `dirent_t` for the format.
// Returns <= -1 on error, read count on success.
// long(file_t fd, void *read_buf, long read_len)
SYSCALL_DEF(0x0304, SYSCALL_FS_GETDENTS)

// RESERVED FOR: Rename and/or move a file to another path, optionally relative to one or two directories.
SYSCALL_DEF(0x0305, SYSCALL_FS_RENAME)

// RESERVED FOR: Get file status given file handler or path, optionally following the final symlink.
SYSCALL_DEF(0x0306, SYSCALL_FS_STAT)



/* ==== PROCESS MANAGEMENT SYSCALLS ==== */



#ifndef __ASSEMBLER__
}
syscall_num_t;
#endif
