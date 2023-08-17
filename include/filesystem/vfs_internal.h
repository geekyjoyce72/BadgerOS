
// SPDX-License-Identifier: MIT

#pragma once

#include "filesystem.h"
#include "filesystem/vfs_types.h"
#include "mutex.h"

// Timeout in microseconds to take on filesystem mutexes.
#define VFS_MUTEX_TIMEOUT 50000

// Index in the VFS table of the filesystem mounted at /.
// Set to -1 if no filesystem is mounted at /.
// If no filesystem is mounted at /, the FS API will not work.
extern ptrdiff_t vfs_root_index;
// Table of mounted filesystems.
extern vfs_t     vfs_table[FILESYSTEM_MOUNT_MAX];
// Mutex for filesystem mounting / unmounting.
// Taken exclusively during mount / unmount operations.
// Taken shared during filesystem access.
extern mutex_t   vfs_mount_mtx;
// Mutex for creating and destroying directory and file handles.
// Taken exclusively when a handle is created or destroyed.
// Taken shared when a handle is used.
extern mutex_t   vfs_handle_mtx;

// List of open shared file handles.
extern vfs_file_shared_t **vfs_file_shared_list;
// Number of open shared file handles.
extern size_t              vfs_file_shared_list_len;
// Capacity of open shared file handles list.
extern size_t              vfs_file_shared_list_cap;

// List of open file handles.
extern vfs_file_handle_t *vfs_file_handle_list;
// Number of open file handles.
extern size_t             vfs_file_handle_list_len;
// Capacity of open file handles list.
extern size_t             vfs_file_handle_list_cap;

// List of open shared directory handles.
extern vfs_dir_shared_t **vfs_dir_shared_list;
// Number of open shared directory handles.
extern size_t             vfs_dir_shared_list_len;
// Capacity of open shared directory handles list.
extern size_t             vfs_dir_shared_list_cap;

// List of open directory handles.
extern vfs_dir_handle_t *vfs_dir_handle_list;
// Number of open directory handles.
extern size_t            vfs_dir_handle_list_len;
// Capacity of open directory handles list.
extern size_t            vfs_dir_handle_list_cap;



/* ==== Thread-unsafe functions ==== */

// Find a shared directory handle by inode, if any.
ptrdiff_t vfs_dir_by_inode(vfs_t *vfs, inode_t inode);
// Get a directory handle by number.
ptrdiff_t vfs_dir_by_handle(dir_t dirno);
// Create a new directory handle.
// If `shared` is -1, a new shared empty handle is created.
ptrdiff_t vfs_dir_create_handle(ptrdiff_t shared);
// Delete a directory handle.
// If this is the last handle referring to one directory, the shared handle is closed too.
void      vfs_dir_destroy_handle(ptrdiff_t handle);

// Find a shared file handle by inode, if any.
ptrdiff_t vfs_file_by_inode(vfs_t *vfs, inode_t inode);
// Get a file handle by number.
ptrdiff_t vfs_file_by_handle(file_t fileno);
// Create a new file handle.
// If `shared` is -1, a new shared empty handle is created.
ptrdiff_t vfs_file_create_handle(ptrdiff_t shared);
// Delete a file handle.
// If this is the last handle referring to one file, the shared handle is closed too.
void      vfs_file_destroy_handle(ptrdiff_t handle);



/* ==== Thread-safe functions ==== */

// Walk to the parent directory of a given path.
// Opens a new vfs_dir_t handle for the directory where the current entry is that of the file.
// Returns NULL on error or if the path refers to the root directory.
vfs_dir_handle_t *vfs_walk(badge_err_t *ec, char const *path, bool *found_out, bool follow_symlink);

// Insert a new file into the given directory.
// If the file already exists, does nothing.
// If `open` is true, a new handle to the file is opened.
vfs_file_handle_t *vfs_create_file(badge_err_t *ec, vfs_dir_shared_t *dir, char const *name, bool open);
// Insert a new directory into the given directory.
// If the file already exists, does nothing.
// If `open` is true, a new handle to the directory is opened.
vfs_dir_handle_t  *vfs_create_dir(badge_err_t *ec, vfs_dir_shared_t *dir, char const *name, bool open);

// Open a directory for reading given parent directory handle.
// If `parent` is NULL, open the root directory.
void     vfs_dir_open(badge_err_t *ec, vfs_dir_handle_t *parent, vfs_dir_shared_t *dir);
// Close a directory opened by `fs_dir_open`.
// Only raises an error if `dir` is an invalid directory descriptor.
void     vfs_dir_close(badge_err_t *ec, vfs_dir_shared_t *dir);
// Read the current directory entry (but not the filename).
// See also: `vfs_dir_read_name` and `vfs_dir_next`.
dirent_t vfs_dir_read_ent(badge_err_t *ec, vfs_dir_shared_t *dir, filesize_t offset);
// Read the current directory entry (only the null-terminated filename).
// Returns the string length of the filename.
// See also: `vfs_dir_read_ent` and `vfs_dir_next`.
size_t   vfs_dir_read_name(badge_err_t *ec, vfs_dir_shared_t *dir, filesize_t offset, char *buf, size_t buf_len);
// Advance to the next directory entry.
// Returns whether a new entry was successfully read.
// See also: `vfs_dir_read_name` and `vfs_dir_read_end`.
bool     vfs_dir_next(badge_err_t *ec, vfs_dir_shared_t *dir, filesize_t *offset);

// Open a file for reading and/or writing given parent directory handle.
void vfs_file_open(badge_err_t *ec, vfs_dir_handle_t *dir, vfs_file_shared_t *file, oflags_t oflags);
// Clone a file opened by `vfs_file_open`.
// Only raises an error if `file` is an invalid file descriptor.
void vfs_file_close(badge_err_t *ec, vfs_file_shared_t *file);
// Read bytes from a file.
void vfs_file_read(badge_err_t *ec, vfs_file_shared_t *file, filesize_t offset, uint8_t *readbuf, filesize_t readlen);
// Write bytes to a file.
void vfs_file_write(
    badge_err_t *ec, vfs_file_shared_t *file, filesize_t offset, uint8_t const *writebuf, filesize_t writelen
);
// Change the length of a file opened by `vfs_file_open`.
void vfs_file_resize(badge_err_t *ec, vfs_file_shared_t *file, filesize_t new_size);

// Commit all pending writes to disk.
// The filesystem, if it does caching, must always sync everything to disk at once.
void vfs_flush(badge_err_t *ec, vfs_t *vfs);
