
// SPDX-License-Identifier: MIT

#pragma once

#include "badge_err.h"
#include "blockdevice.h"

// Maximum number of mountable filesystems.
#define FILESYSTEM_MOUNT_MAX   4
// Maximum supported filename length.
#define FILESYSTEM_NAME_MAX    255
// Maximum supported path length.
#define FILESYSTEM_PATH_MAX    511
// Maximum supported symlink nesting.
#define FILESYSTEM_SYMLINK_MAX 8
// Maximum supported directory entries.
#define FILESYSTEM_DIRENT_MAX  64

// Mount as read-only filesystem (default: read-write).
#define MOUNTFLAGS_READONLY 0x00000001
// Filesystem mounting flags.
typedef uint32_t mountflags_t;

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
#define OFLAGS_ECLUSIVE  0x00000020
// Do not inherit to child process.
#define OFLAGS_CLOEXEC   0x00000040
// File opening mode flags.
typedef uint32_t oflags_t;

// Value used for absent inodes.
#define INODE_NONE ((inode_t)0)
// Type used for inode numbers.
typedef long inode_t;

// Value used for absent file handle.
#define FILE_NONE ((file_t)-1)
// Type used for file handles in the kernel.
typedef int          file_t;
// Type used for file offsets.
typedef int          fileoff_t;
// Type used for file sizes.
typedef unsigned int filesize_t;

// Value used for absent directory handle.
#define DIR_NONE ((dir_t)-1)
// Type used for directory handles in the kernel.
typedef int dir_t;

// Supported filesystem types.
typedef enum {
    // Auto-detect filesystem type.
    FS_TYPE_AUTO,
    // FAT12, FAT16 or FAT32.
    FS_TYPE_FAT,
} fs_type_t;

// Modes for VFS seek.
typedef enum {
    // Seek from start of file.
    SEEK_ABS = -1,
    // Seek from current position.
    SEEK_CUR = 0,
    // Seek from end of file.
    SEEK_END = 1,
} fs_seek_t;

// Directory entry.
typedef struct {
    // Inode number; gauranteed to be unique per physical file per filesystem.
    inode_t    inode;
    // Node is a directory.
    bool       is_dir;
    // Node is a symbolic link.
    bool       is_symlink;
    // UNIX permission flags.
    uint16_t   perms;
    // Length of the filename.
    filesize_t name_len;
} dirent_t;

// Try to mount a filesystem.
void      fs_mount(badge_err_t *ec, fs_type_t type, blkdev_t *media, char const *mountpoint, mountflags_t flags);
// Unmount a filesystem.
// Only raises an error if there isn't a valid filesystem to unmount.
void      fs_umount(badge_err_t *ec, char const *mountpoint);
// Try to identify the filesystem stored in the block device
// Returns `FS_TYPE_AUTO` on error or if the filesystem is unknown.
fs_type_t fs_detect(badge_err_t *ec, blkdev_t *media);

// Open a directory for reading.
dir_t    fs_dir_open(badge_err_t *ec, char const *path);
// Close a directory opened by `fs_dir_open`.
// Only raises an error if `dir` is an invalid directory descriptor.
void     fs_dir_close(badge_err_t *ec, dir_t dir);
// Read the current directory entry (but not the filename).
// Only moves to the next entry if `next` is true.
// See also: `fs_dir_read_name`.
dirent_t fs_dir_read_ent(badge_err_t *ec, dir_t dir, bool next);
// Read the current directory entry (only the null-terminated filename).
// Returns the string length of the filename.
// Only moves to the next entry if `next` is true.
// See also: `fs_dir_read_ent`.
size_t   fs_dir_read_name(badge_err_t *ec, dir_t dir, char *buf, size_t buf_len, bool next);

// Open a file for reading and/or writing.
file_t     fs_open(badge_err_t *ec, char const *path, oflags_t oflags);
// Clone a file opened by `fs_open`.
// Only raises an error if `file` is an invalid file descriptor.
void       fs_close(badge_err_t *ec, file_t file);
// Read bytes from a file.
// Returns the amount of data successfully read.
filesize_t fs_read(badge_err_t *ec, file_t file, uint8_t *readbuf, filesize_t readlen);
// Write bytes to a file.
// Returns the amount of data successfully written.
filesize_t fs_write(badge_err_t *ec, file_t file, uint8_t const *writebuf, filesize_t writelen);
// Get the current offset in the file.
fileoff_t  fs_tell(badge_err_t *ec, file_t file);
// Set the current offset in the file.
// Returns the new offset in the file.
fileoff_t  fs_seek(badge_err_t *ec, file_t file, fileoff_t off, fs_seek_t seekmode);
// Force any write caches to be flushed for a given file.
// If the file is `FILE_NONE`, all open files are flushed.
void       fs_flush(badge_err_t *ec, file_t file);
