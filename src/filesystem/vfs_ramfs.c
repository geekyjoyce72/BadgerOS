
// SPDX-License-Identifier: MIT

#include "filesystem/vfs_ramfs.h"

#include "assertions.h"
#include "badge_strings.h"
#include "malloc.h"



// Atomically allocate memory within the quota.
static void *ramfs_malloc(badge_err_t *ec, vfs_t *fs, size_t cap) {
    // TODO.
    return malloc(cap);
}

// Atomically resize memory within the quota.
static void *ramfs_realloc(badge_err_t *ec, vfs_t *fs, void *mem, size_t cap) {
    // TODO.
    return realloc(mem, cap);
}

// Atomically free memory within the quota.
static void ramfs_free(vfs_t *fs, size_t cap) {
    // TODO.
    free(cap);
}



// Try to resize an inode.
static bool inode_resize(badge_err_t *ec, vfs_t *vfs, vfs_ramfs_inode_t *inode, fileoff_t size) {
    assert_dev_drop(size >= 0);

    if (inode->cap >= 2 * size) {
        // If capacity is too large, try to save some memory.
        void *mem = ramfs_realloc(ec, vfs, inode->buf, inode->cap / 2);
        if (mem) {
            inode->cap /= 2;
            inode->buf = mem;
        }
        inode->len = size;
        return true;

    } else if (inode->cap >= size) {
        // If capacity is large enough, don't bother resizing.
        inode->len = size;
        return true;

    } else if (inode->cap < size) {
        // If the capacity is *not* large enough, increase the size.
        size_t cap = 1;
        while (cap < size) {
            cap *= 2;
        }
        void *mem = ramfs_realloc(ec, vfs, inode->buf, cap);
        if (mem) {
            inode->cap = cap;
            inode->buf = mem;
            inode->len = size;
            return true;
        } else {
            return false;
        }
    }
}



// Try to mount a ramfs filesystem.
void vfs_ramfs_mount(badge_err_t *ec, vfs_t *vfs) {
    // RAMFS does not use a block device.
    if (vfs->media) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_PARAM);
        return;
    }

    // TODO: Parameters.
    atomic_store_explicit(&vfs->ramfs.ram_usage, 0, memory_order_relaxed);
    vfs->ramfs.ram_limit      = 65536;
    vfs->ramfs.inode_list_len = 32;
    vfs->ramfs.inode_list     = ramfs_malloc(ec, vfs, sizeof(*vfs->ramfs.inode_list) * vfs->ramfs.inode_list_len);
    if (!badge_err_is_ok(ec))
        return;
    vfs->ramfs.inode_usage = ramfs_malloc(ec, vfs, sizeof(*vfs->ramfs.inode_usage) * vfs->ramfs.inode_list_len);
    if (!badge_err_is_ok(ec)) {
        ramfs_free(vfs, vfs->ramfs.inode_list);
        return;
    }
    mutex_init_shared(ec, &vfs->ramfs.mtx);

    // Set inode usage.
    mem_set(vfs->ramfs.inode_usage, false, vfs->ramfs.inode_list_len - 2);
}

// Unmount a ramfs filesystem.
void vfs_ramfs_umount(vfs_t *vfs) {
    mutex_destroy(NULL, &vfs->ramfs.mtx);
    ramfs_free(vfs, vfs->ramfs.inode_list);
    ramfs_free(vfs, vfs->ramfs.inode_usage);
}



// Find an empty inode.
static ptrdiff_t find_inode(vfs_t *vfs) {
    for (ptrdiff_t i = 3; i < vfs->ramfs.inode_list_len; i++) {
        if (!vfs->ramfs.inode_usage[i])
            return i;
    }
    return -1;
}

// Insert a new directory entry.
static void insert_dirent(vfs_t *vfs, vfs_ramfs_inode_t *dir, char const *name, inode_t inode) {
}

// Insert a new file into the given directory.
// If the file already exists, does nothing.
void vfs_ramfs_create_file(badge_err_t *ec, vfs_t *vfs, vfs_file_handle_t *dir, char const *name) {
    if (!mutex_acquire(ec, &vfs->ramfs.mtx, VFS_MUTEX_TIMEOUT))
        return;

    // Find a vacant inode to assign.
    ptrdiff_t inum = find_inode(vfs);
    if (inum == -1) {
        mutex_release(NULL, &vfs->ramfs.mtx);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_NOSPACE);
        return;
    }

    // TODO: Write a directory entry.

    mutex_release(NULL, &vfs->ramfs.mtx);
    badge_err_set_ok(ec);
}

// Insert a new directory into the given directory.
// If the file already exists, does nothing.
void vfs_ramfs_create_dir(badge_err_t *ec, vfs_t *vfs, vfs_file_handle_t *dir, char const *name) {
}

// Unlink a file from the given directory.
// If this is the last reference to an inode, the inode is deleted.
void vfs_ramfs_unlink(badge_err_t *ec, vfs_t *vfs, vfs_file_handle_t *dir, char const *name) {
}



// Atomically read all directory entries and cache them into the directory handle.
// Refer to `dirent_t` for the structure of the cache.
void vfs_ramfs_dir_read(badge_err_t *ec, vfs_t *vfs, vfs_file_handle_t *dir);
// Open a file for reading and/or writing.
void vfs_ramfs_file_open(badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *file, char const *path, oflags_t oflags);
// Clone a file opened by `vfs_ramfs_file_open`.
// Only raises an error if `file` is an invalid file descriptor.
void vfs_ramfs_file_close(badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *file);
// Read bytes from a file.
void vfs_ramfs_file_read(
    badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *file, fileoff_t offset, uint8_t *readbuf, fileoff_t readlen
);
// Write bytes from a file.
void vfs_ramfs_file_write(
    badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *file, fileoff_t offset, uint8_t const *writebuf, fileoff_t writelen
);
// Change the length of a file opened by `vfs_ramfs_file_open`.
void vfs_ramfs_file_resize(badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *file, fileoff_t new_size);

// Commit all pending writes to disk.
// The filesystem, if it does caching, must always sync everything to disk at once.
void vfs_ramfs_flush(badge_err_t *ec, vfs_ramfs_t *vfs);
