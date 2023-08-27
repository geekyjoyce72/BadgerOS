
// SPDX-License-Identifier: MIT

#pragma once

#include "assertions.h"
#include "attributes.h"
#include "filesystem.h"
#include "mutex.h"

#include <stdatomic.h>

/* ==== In-memory structures ==== */

// File data storage.
typedef struct {
    // Data buffer length.
    size_t   len;
    // Data buffer capacity.
    size_t   cap;
    // Data buffer.
    char    *buf;
    // Inode number.
    inode_t  inode;
    // File type and protection.
    uint16_t mode;
    // Number of hard links.
    size_t   links;
    // Owner user ID.
    int      uid;
    // Owner group ID.
    int      gid;
} vfs_ramfs_inode_t;

// RAMFS directory entry.
typedef struct {
    // Entry size.
    fileoff_t size;
    // Node is a directory.
    bool      is_dir;
    // Node is a symbolic link.
    bool      is_symlink;
    // Name length.
    uint8_t   name_len;
} ramfs_dirent_t;

// RAM filesystem file / directory handle.
// This handle is shared between multiple holders of the same file.
typedef vfs_ramfs_inode_t *vfs_ramfs_file_t;

// Mounted RAM filesystem.
typedef struct {
    // RAM limit for the entire filesystem.
    size_t             ram_limit;
    // RAM usage.
    atomic_size_t      ram_usage;
    // Inode table, indices 0 and 1 are unused.
    vfs_ramfs_inode_t *inode_list;
    // Inode table usage map.
    bool              *inode_usage;
    // Number of allocated inodes.
    size_t             inode_list_len;
    // THE RAMFS mutex.
    // Acquired shared for all read-only operations.
    // Acquired exclusive for any write operation.
    mutex_t            mtx;
} vfs_ramfs_t;
