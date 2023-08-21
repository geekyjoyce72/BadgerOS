
// SPDX-License-Identifier: MIT

#pragma once

#include "assertions.h"
#include "attributes.h"
#include "filesystem.h"

#include <stdatomic.h>

/* ==== In-memory structures ==== */

// RAMFS directory entry.
typedef struct {
    // Node is a directory.
    bool is_dir;
    // Node is a symbolic link.
    bool is_symlink;
} ramfs_dirent_t;

// RAM filesystem file / directory handle.
// This handle is shared between multiple holders of the same file.
typedef struct {
    // Data buffer length.
    size_t len;
    // Data buffer capacity.
    size_t cap;
    // Data buffer.
    char  *buf;
    // The inode data.
    stat_t stat;
} vfs_ramfs_file_t;

// Mounted RAM filesystem.
typedef struct {
    // RAM limit for the entire filesystem.
    size_t            ram_limit;
    // RAM usage.
    atomic_size_t     ram_usage;
    // Inode table.
    vfs_ramfs_file_t *inode_list;
    // Number of allocated inodes.
    size_t            inode_list_len;
} vfs_ramfs_t;
