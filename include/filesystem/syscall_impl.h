
// SPDX-License-Identifier: MIT

#include "filesystem.h"
#include "syscall.h"

// Open a file, optionally relative to a directory.
// Returns <= -1 on error, file descriptor number of success.
int syscall_fs_open(char const *path, int relative_to, int oflags);

// Flush and close a file.
bool syscall_fs_close(int fd);

// Read bytes from a file.
// Returns -1 on EOF, <= -2 on error, read count on success.
long syscall_fs_read(int fd, void *read_buf, long read_len);

// Write bytes to a file.
// Returns <= -1 on error, write count on success.
long syscall_fs_write(int fd, void const *write_buf, long write_len);

// Read directory entries from a directory handle.
// See `dirent_t` for the format.
// Returns <= -1 on error, read count on success.
long syscall_fs_getdents(int fd, void *read_buf, long read_len);
