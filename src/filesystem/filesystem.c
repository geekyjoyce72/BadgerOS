
// SPDX-License-Identifier: MIT

#include "filesystem.h"

// Open a file for reading and optionally writing.
file_t     vfs_open(badge_err_t *ec, char const *path, uint32_t oflags);
// Close a previously opened file.
void       vfs_close(file_t file);
// Read bytes from a file.
// Returns the amount of data successfully read.
filesize_t vfs_read(badge_err_t *ec, uint8_t *readbuf, filesize_t readlen);
// Write bytes from a file.
// Returns the amount of data successfully written.
filesize_t vfs_write(badge_err_t *ec, uint8_t const *writebuf, filesize_t writelen);
// Force any write caches to be flushed for a given file.
// If the file is `FILE_NULL`, all open files are flushed.
void       vfs_flush(badge_err_t *ec, file_t file);
