
// SPDX-License-Identifier: MIT

#pragma once

#include "blockdevice.h"
#include "filesystem.h"
#include "filesystem/vfs_fat_types.h"
#include "mutex.h"

typedef struct vfs vfs_t;

// VFS shared opened directory handle.
// Shared between all directory handles referring to the same directory.
typedef struct {
    // Reference count.
    size_t     refcount;
    // Filesystem-specific encoding of the directory size.
    filesize_t size;
    // Filesystem-specific information.
    union {
        // FAT12, FAT16 or FAT32.
        vfs_fat_dir_t fat_dir;
    };
    // Inode number (gauranteed to be unique per VFS).
    inode_t inode;
    // Pointer to the VFS on which this directory exists.
    vfs_t  *vfs;
} vfs_dir_shared_t;

// VFS opened directory handle.
typedef struct {
    // Filesystem-specific current access positon.
    // Note: Must be bounds-checked on every file I/O.
    filesize_t        offset;
    // Pointer to shared file handle.
    vfs_dir_shared_t *shared;
    // Handle number.
    dir_t             dirno;
} vfs_dir_handle_t;

// VFS shared opened file handle.
// Shared between all file handles referring to the same file.
typedef struct {
    // Reference count.
    size_t     refcount;
    // Current file size.
    filesize_t size;
    // Filesystem-specific information.
    union {
        // FAT12, FAT16 or FAT32.
        vfs_fat_file_t fat_file;
    };
    // Inode number (gauranteed to be unique per VFS).
    inode_t inode;
    // Pointer to the VFS on which this file exists.
    vfs_t  *vfs;
} vfs_file_shared_t;

// VFS opened file handle.
typedef struct {
    // Current access position.
    // Note: Must be bounds-checked on every file I/O.
    filesize_t         offset;
    // File is writeable.
    bool               write;
    // File is readable.
    bool               read;
    // Pointer to shared file handle.
    vfs_file_shared_t *shared;
    // Handle number.
    file_t             fileno;
} vfs_file_handle_t;

// VFS mounted filesystem.
struct vfs {
    // Copy of mount point.
    char     *mountpoint;
    // Read-only flag.
    bool      readonly;
    // Associated block device.
    blkdev_t *media;
    // Filesystem type.
    fs_type_t type;
    // Filesystem-specific information.
    union {
        // FAT12, FAT16 or FAT32.
        vfs_fat_t fat;
    };
};
