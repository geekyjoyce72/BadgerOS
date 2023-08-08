
// SPDX-License-Identifier: MIT

#include "filesystem/vfs_fat.h"

// Try to mount a FAT filesystem.
void vfs_fat_mount(badge_err_t *ec, vfs_t *vfs);

// Unmount a FAT filesystem.
void vfs_fat_umount(vfs_t *vfs);

// Identify whether a block device contains a FAT filesystem.
// Returns false on error.
bool vfs_fat_detect(badge_err_t *ec, blkdev_t *dev) {
    badge_err_t ec0;
    if (!ec)
        ec = &ec0;

    // Read BPB.
    fat_bpb_t bpb;
    blkdev_read_partial(ec, dev, 0, 0, (void *)&bpb, sizeof(fat_bpb_t));
    if (!badge_err_is_ok(ec))
        return false;

    // Checked signature #1: valid sector size.
    // Sector size is an integer power of 2 from 512 to 4096.
    if (bpb.bytes_per_sector < 512 || bpb.bytes_per_sector > 4096) {
        return false;
    }
    if (bpb.bytes_per_sector & (bpb.bytes_per_sector - 1)) {
        return false;
    }

    // Checked signature #2: FAT BPB signature bytes.
    uint8_t tmp[2];
    blkdev_read_partial(ec, dev, 0, 510, tmp, sizeof(tmp));
    if (!badge_err_is_ok(ec))
        return false;
    if (tmp[0] != 0x55 || tmp[1] != 0xAA) {
        return false;
    }

    // Both minor signature checks passed, this filesystem is probably FAT.
    return true;
}



// Open a directory for reading.
void     vfs_fat_dir_open(badge_err_t *ec, vfs_dir_shared_t *dir, char const *path);
// Close a directory opened by `fs_dir_open`.
// Only raises an error if `dir` is an invalid directory descriptor.
void     vfs_fat_dir_close(badge_err_t *ec, vfs_dir_shared_t *dir);
// Read the current directory entry (but not the filename).
// See also: `vfs_fat_dir_read_name` and `vfs_fat_dir_next`.
dirent_t vfs_fat_dir_read_ent(badge_err_t *ec, vfs_dir_shared_t *dir, filesize_t offset);
// Read the current directory entry (only the null-terminated filename).
// Returns the string length of the filename.
// See also: `vfs_fat_dir_read_ent` and `vfs_fat_dir_next`.
size_t   vfs_fat_dir_read_name(badge_err_t *ec, vfs_dir_shared_t *dir, filesize_t offset, char *buf, size_t buf_len);
// Advance to the next directory entry.
// Returns whether a new entry was successfully read.
// See also: `vfs_fat_dir_read_name` and `vfs_fat_dir_read_end`.
bool     vfs_fat_dir_next(badge_err_t *ec, vfs_dir_shared_t *dir, filesize_t *offset);

// Open a file for reading and/or writing.
void vfs_fat_file_open(badge_err_t *ec, vfs_file_shared_t *file, char const *path, oflags_t oflags);
// Clone a file opened by `vfs_fat_file_open`.
// Only raises an error if `file` is an invalid file descriptor.
void vfs_fat_file_close(badge_err_t *ec, vfs_file_shared_t *file);
// Read bytes from a file.
void vfs_fat_file_read(
    badge_err_t *ec, vfs_file_shared_t *file, filesize_t offset, uint8_t *readbuf, filesize_t readlen
);
// Write bytes from a file.
void vfs_fat_file_write(
    badge_err_t *ec, vfs_file_shared_t *file, filesize_t offset, uint8_t const *writebuf, filesize_t writelen
);
// Change the length of a file opened by `vfs_fat_file_open`.
void vfs_fat_file_resize(badge_err_t *ec, vfs_file_shared_t *file, filesize_t new_size);

// Commit all pending writes to disk.
// The filesystem, if it does caching, must always sync everything to disk at once.
void vfs_fat_flush(badge_err_t *ec, vfs_fat_t *vfs);
