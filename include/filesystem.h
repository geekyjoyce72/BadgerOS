
// SPDX-License-Identifier: MIT

#pragma once

#include "badge_err.h"

// Value used for absent file descriptor.
#define FILE_NULL ((file_t)0)
// Type used for file handles in the kernel.
typedef int file_t;
// Type used for file offsets.
typedef int fileoff_t;
// Type used for file sizes.
typedef unsigned int filesize_t;

// Supported filesystem types.
typedef enum {
    // Microsoft FAT filesystem.
    FS_TYPE_FAT,
} fs_type_t;

// Open a file for reading and optionally writing.
file_t vfs_open(badge_err_t *ec, char const *path, uint32_t oflags);
// Close a previously opened file.
void vfs_close(file_t file);
// Read bytes from a file.
// Returns the amount of data successfully read.
filesize_t vfs_read(badge_err_t *ec, uint8_t *readbuf, filesize_t readlen);
// Write bytes from a file.
// Returns the amount of data successfully written.
filesize_t vfs_write(badge_err_t *ec, uint8_t const *writebuf, filesize_t writelen);
// Force any write caches to be flushed for a given file.
// If the file is `FILE_NULL`, all open files are flushed.
void vfs_flush(badge_err_t *ec, file_t file);
