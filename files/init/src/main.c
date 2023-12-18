
#include "syscall.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern void syscall_temp_write(char const *data, size_t length);

// Open for read-only.
#define OFLAGS_READONLY  0x00000001
// Open for write-only.
#define OFLAGS_WRITEONLY 0x00000002
// Open for read and write.
#define OFLAGS_READWRITE 0x00000003
// Seek to the end on opening.
#define OFLAGS_APPEND    0x00000004
// Truncate on opening.
#define OFLAGS_TRUNCATE  0x00000008
// Create if it doesn't exist on opening.
#define OFLAGS_CREATE    0x00000010
// Error if it exists on opening.
#define OFLAGS_EXCLUSIVE 0x00000020
// Do not inherit to child process.
#define OFLAGS_CLOEXEC   0x00000040
// Open a directory instead of a file.
#define OFLAGS_DIRECTORY 0x00000080

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



size_t cstr_length(char const *const cstr) {
    char const *ptr = cstr;
    while (*ptr) ptr++;
    return ptr - cstr;
}

void print(char const *message) {
    syscall_temp_write(message, cstr_length(message));
}



int main(int argc, char **argv) {
    char buf[128];
    print("Hello World from C???\n");
    int fd = syscall_fs_open("/etc/motd", 0, OFLAGS_READONLY);
    if (fd < 0)
        print("No FD :c\n");
    long count = syscall_fs_read(fd, buf, sizeof(buf));
    syscall_fs_close(fd);
    if (count > 0)
        syscall_temp_write(buf, count);
    else
        print("No read :c\n");
    return 0;
}
