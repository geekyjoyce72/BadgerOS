
// SPDX-License-Identifier: MIT

#include "filesystem/vfs_ramfs.h"

// Try to mount a ramfs filesystem.
void vfs_ramfs_mount(badge_err_t *ec, vfs_t *vfs) {
    // TODO: Parameters.
}
// Unmount a ramfs filesystem.
void vfs_ramfs_umount(vfs_t *vfs);

// Atomically read all directory entries and cache them into the directory handle.
// Refer to `dirent_t` for the structure of the cache.
void vfs_ramfs_dir_read(badge_err_t *ec, vfs_file_handle_t *dir);
// Open a file for reading and/or writing.
void vfs_ramfs_file_open(badge_err_t *ec, vfs_file_shared_t *file, char const *path, oflags_t oflags);
// Clone a file opened by `vfs_ramfs_file_open`.
// Only raises an error if `file` is an invalid file descriptor.
void vfs_ramfs_file_close(badge_err_t *ec, vfs_file_shared_t *file);
// Read bytes from a file.
void vfs_ramfs_file_read(
    badge_err_t *ec, vfs_file_shared_t *file, fileoff_t offset, uint8_t *readbuf, fileoff_t readlen
);
// Write bytes from a file.
void vfs_ramfs_file_write(
    badge_err_t *ec, vfs_file_shared_t *file, fileoff_t offset, uint8_t const *writebuf, fileoff_t writelen
);
// Change the length of a file opened by `vfs_ramfs_file_open`.
void vfs_ramfs_file_resize(badge_err_t *ec, vfs_file_shared_t *file, fileoff_t new_size);

// Commit all pending writes to disk.
// The filesystem, if it does caching, must always sync everything to disk at once.
void vfs_ramfs_flush(badge_err_t *ec, vfs_ramfs_t *vfs);
