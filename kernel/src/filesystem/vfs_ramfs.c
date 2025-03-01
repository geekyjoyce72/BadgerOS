
// SPDX-License-Identifier: MIT

#include "filesystem/vfs_ramfs.h"

#include "assertions.h"
#include "badge_strings.h"
#include "malloc.h"



// Try to resize an inode.
static bool resize_inode(badge_err_t *ec, vfs_t *vfs, vfs_ramfs_inode_t *inode, size_t size) {
    (void)vfs;

    if (inode->cap >= 2 * size) {
        // If capacity is too large, try to save some memory.
        size_t cap = inode->cap / 2;
        void  *mem = realloc(inode->buf, inode->cap / 2);
        if (mem || cap == 0) {
            inode->cap /= 2;
            inode->buf  = mem;
        }
        inode->len = size;
        return true;

    } else if (inode->cap >= size) {
        // If capacity is large enough, don't bother resizing.
        inode->len = size;
        badge_err_set_ok(ec);
        return true;

    } else {
        // If the capacity is not large enough, increase the size.
        size_t cap = 1;
        while (cap < size) {
            cap *= 2;
        }
        void *mem = realloc(inode->buf, cap);
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

// Find an empty inode.
static ptrdiff_t find_inode(vfs_t *vfs) {
    for (size_t i = VFS_RAMFS_INODE_FIRST; i < vfs->ramfs.inode_list_len; i++) {
        if (!vfs->ramfs.inode_usage[i])
            return (ptrdiff_t)i;
    }
    return -1;
}

// Decrease the refcount of an inode and delete it if it reaches 0.
static void pop_inode_refcount(vfs_t *vfs, vfs_ramfs_inode_t *inode) {
    inode->links--;
    if (inode->links == 0) {
        // Free inode.
        free(inode->buf);
        vfs->ramfs.inode_usage[inode->inode] = false;
    }
}

// Insert a new directory entry.
static bool insert_dirent(badge_err_t *ec, vfs_t *vfs, vfs_ramfs_inode_t *dir, vfs_ramfs_dirent_t *ent) {
    // Allocate space in the directory.
    size_t pre_size = dir->len;
    if (!resize_inode(ec, vfs, dir, pre_size + ent->size)) {
        return false;
    }

    // Copy to the destination.
    mem_copy(dir->buf + pre_size, ent, ent->size);
    return true;
}

// Remove a directory entry.
// Takes a pointer to an entry in the directory's buffer.
static void remove_dirent(vfs_t *vfs, vfs_ramfs_inode_t *dir, vfs_ramfs_dirent_t *ent) {
    size_t off      = (size_t)ent - (size_t)dir->buf;
    size_t ent_size = ent->size;

    // Copy back entries further in.
    mem_copy(dir->buf + off, dir->buf + off + ent_size, dir->len - ent_size);

    // Resize the buffer.
    resize_inode(NULL, vfs, dir, dir->len - ent_size);
}

// Find the directory entry of a given filename in a directory.
// Returns a pointer to an entry in the directory's buffer, or NULL if not found.
static vfs_ramfs_dirent_t *find_dirent(badge_err_t *ec, vfs_t *vfs, vfs_ramfs_inode_t *dir, char const *name) {
    (void)vfs;
    badge_err_set_ok(ec);
    size_t off = 0;
    while (off < dir->len) {
        vfs_ramfs_dirent_t *ent = (vfs_ramfs_dirent_t *)(dir->buf + off);
        if (cstr_equals(name, ent->name)) {
            return ent;
        }
        off += ent->size;
    }
    return NULL;
}

// Insert a new file or directory into the given directory.
// If the file already exists, does nothing.
static vfs_ramfs_inode_t *
    create_file(badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *dir, char const *name, filetype_t type) {
    size_t name_len = cstr_length_upto(name, VFS_RAMFS_NAME_MAX + 1);
    if (name_len > VFS_RAMFS_NAME_MAX) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_TOOLONG);
    }
    assert_always(mutex_acquire(NULL, &vfs->ramfs.mtx, VFS_MUTEX_TIMEOUT));

    // Test whether the file already exists.
    vfs_ramfs_inode_t  *dirptr   = dir->ramfs_file;
    vfs_ramfs_dirent_t *existing = find_dirent(ec, vfs, dirptr, name);
    if (existing) {
        mutex_release(NULL, &vfs->ramfs.mtx);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_EXISTS);
        return NULL;
    }

    // Find a vacant inode to assign.
    ptrdiff_t inum = find_inode(vfs);
    if (inum == -1) {
        mutex_release(NULL, &vfs->ramfs.mtx);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_NOSPACE);
        return NULL;
    }

    // Write a directory entry.
    vfs_ramfs_dirent_t ent = {
        .size     = offsetof(vfs_ramfs_dirent_t, name) + name_len + 1,
        .name_len = name_len,
        .inode    = inum,
    };
    ent.size += (~ent.size + 1) % sizeof(size_t);
    mem_copy(ent.name, name, name_len + 1);

    // Set up inode.
    vfs_ramfs_inode_t *iptr = &vfs->ramfs.inode_list[inum];

    iptr->buf   = NULL;
    iptr->len   = 0;
    iptr->cap   = 0;
    iptr->inode = inum;
    iptr->mode  = (type << VFS_RAMFS_MODE_BIT) | 0777; /* TODO. */
    iptr->links = 1;
    iptr->uid   = 0; /* TODO. */
    iptr->gid   = 0; /* TODO. */

    // Copy into the end of the directory.
    if (insert_dirent(ec, vfs, dirptr, &ent)) {
        // If successful, mark inode as in use.
        vfs->ramfs.inode_usage[inum] = true;
    }

    mutex_release(NULL, &vfs->ramfs.mtx);
    return iptr;
}

// Test whether a directory is empty.
static bool is_dir_empty(vfs_ramfs_inode_t *dir) {
    size_t off = 0;
    while (off < dir->len) {
        vfs_ramfs_dirent_t *ent = (vfs_ramfs_dirent_t *)(dir->buf + off);
        if (!cstr_equals(".", ent->name) && !cstr_equals("..", ent->name)) {
            return false;
        }
        off += ent->size;
    }
    return true;
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
    vfs->type                 = FS_TYPE_RAMFS;
    vfs->ramfs.ram_limit      = 65536;
    vfs->ramfs.inode_list_len = 32;
    vfs->ramfs.inode_list     = malloc(sizeof(*vfs->ramfs.inode_list) * vfs->ramfs.inode_list_len);
    if (!badge_err_is_ok(ec))
        return;
    vfs->ramfs.inode_usage = malloc(sizeof(*vfs->ramfs.inode_usage) * vfs->ramfs.inode_list_len);
    if (!badge_err_is_ok(ec)) {
        free(vfs->ramfs.inode_list);
        return;
    }
    vfs->inode_root = VFS_RAMFS_INODE_ROOT;
    mutex_init(ec, &vfs->ramfs.mtx, true, false);

    // Clear inode usage.
    mem_set(vfs->ramfs.inode_list, 0, sizeof(*vfs->ramfs.inode_list) * vfs->ramfs.inode_list_len);
    mem_set(vfs->ramfs.inode_usage, false, vfs->ramfs.inode_list_len - 2);

    // Create root directory.
    vfs->ramfs.inode_usage[VFS_RAMFS_INODE_ROOT] = true;
    vfs_ramfs_inode_t *iptr                      = &vfs->ramfs.inode_list[VFS_RAMFS_INODE_ROOT];

    iptr->buf   = NULL;
    iptr->len   = 0;
    iptr->cap   = 0;
    iptr->inode = VFS_RAMFS_INODE_ROOT;
    iptr->mode  = (FILETYPE_DIR << VFS_RAMFS_MODE_BIT) | 0777; /* TODO. */
    iptr->links = 1;
    iptr->uid   = 0; /* TODO. */
    iptr->gid   = 0; /* TODO. */

    vfs_ramfs_dirent_t ent = {
        .size     = sizeof(ent) - VFS_RAMFS_NAME_MAX - 1 + sizeof(size_t),
        .inode    = VFS_RAMFS_INODE_ROOT,
        .name_len = 1,
        .name     = {'.', 0},
    };
    insert_dirent(ec, vfs, iptr, &ent);
    if (!badge_err_is_ok(ec)) {
        free(iptr->buf);
        free(vfs->ramfs.inode_list);
        free(vfs->ramfs.inode_usage);
        mutex_destroy(NULL, &vfs->ramfs.mtx);
    }

    ent.name_len = 2;
    ent.name[1]  = '.';
    ent.name[2]  = 0;
    insert_dirent(ec, vfs, iptr, &ent);
    if (!badge_err_is_ok(ec)) {
        free(iptr->buf);
        free(vfs->ramfs.inode_list);
        free(vfs->ramfs.inode_usage);
        mutex_destroy(NULL, &vfs->ramfs.mtx);
    }
}

// Unmount a ramfs filesystem.
void vfs_ramfs_umount(vfs_t *vfs) {
    mutex_destroy(NULL, &vfs->ramfs.mtx);
    free(vfs->ramfs.inode_list);
    free(vfs->ramfs.inode_usage);
}



// Insert a new file into the given directory.
// If the file already exists, does nothing.
void vfs_ramfs_create_file(badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *dir, char const *name) {
    create_file(ec, vfs, dir, name, FILETYPE_REG);
}

// Insert a new directory into the given directory.
// If the file already exists, does nothing.
void vfs_ramfs_create_dir(badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *dir, char const *name) {
    badge_err_t ec0;
    if (!ec)
        ec = &ec0;

    // Create new directory.
    vfs_ramfs_inode_t *iptr = create_file(ec, vfs, dir, name, FILETYPE_DIR);
    if (!badge_err_is_ok(ec))
        return;

    // Write . and .. entries.
    vfs_ramfs_dirent_t ent = {
        .size     = sizeof(ent) - VFS_RAMFS_NAME_MAX - 1 + sizeof(size_t),
        .inode    = iptr->inode,
        .name_len = 1,
        .name     = {'.', 0},
    };
    insert_dirent(ec, vfs, iptr, &ent);
    if (!badge_err_is_ok(ec)) {
        pop_inode_refcount(vfs, iptr);
        vfs_ramfs_dirent_t *ent = find_dirent(ec, vfs, dir->ramfs_file, name);
        assert_dev_drop(ent != NULL);
        remove_dirent(vfs, dir->ramfs_file, ent);
        return;
    }

    ent.name_len = 2;
    ent.name[1]  = '.';
    ent.name[2]  = 0;
    ent.inode    = dir->inode;
    insert_dirent(ec, vfs, iptr, &ent);
    if (!badge_err_is_ok(ec)) {
        pop_inode_refcount(vfs, iptr);
        vfs_ramfs_dirent_t *ent = find_dirent(ec, vfs, dir->ramfs_file, name);
        assert_dev_drop(ent != NULL);
        remove_dirent(vfs, dir->ramfs_file, ent);
        return;
    }
}

// Unlink a file from the given directory.
// If this is the last reference to an inode, the inode is deleted.
void vfs_ramfs_unlink(badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *dir, char const *name) {
    size_t name_len = cstr_length_upto(name, VFS_RAMFS_NAME_MAX + 1);
    if (name_len > VFS_RAMFS_NAME_MAX) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_TOOLONG);
    }

    // The . and .. entries can not be removed.
    if (cstr_equals(name, ".") || cstr_equals(name, "..")) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_PARAM);
        return;
    }

    assert_always(mutex_acquire(ec, &vfs->ramfs.mtx, VFS_MUTEX_TIMEOUT));

    // Find the directory entry with the given name.
    vfs_ramfs_inode_t  *dirptr = dir->ramfs_file;
    vfs_ramfs_dirent_t *ent    = find_dirent(ec, vfs, dirptr, name);
    if (!ent) {
        mutex_release(NULL, &vfs->ramfs.mtx);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_NOTFOUND);
        return;
    }


    // If it is also a directory, assert that it is empty.
    vfs_ramfs_inode_t *iptr = &vfs->ramfs.inode_list[ent->inode];
    if ((iptr->mode & VFS_RAMFS_MODE_MASK) == FILETYPE_DIR << VFS_RAMFS_MODE_BIT) {
        // Directories that are not empty cannot be removed.
        if (!is_dir_empty(iptr)) {
            badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_NOTEMPTY);
            mutex_release(NULL, &vfs->ramfs.mtx);
            return;
        }
    }

    // Decrease inode refcount.
    pop_inode_refcount(vfs, iptr);

    // Remove directory entry.
    remove_dirent(vfs, dirptr, ent);

    mutex_release(NULL, &vfs->ramfs.mtx);
}

// Test for the existence of a file in the given directory.
bool vfs_ramfs_exists(badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *dir, char const *name) {
    size_t name_len = cstr_length_upto(name, VFS_RAMFS_NAME_MAX + 1);
    if (name_len > VFS_RAMFS_NAME_MAX) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_TOOLONG);
    }

    assert_always(mutex_acquire_shared(ec, &vfs->ramfs.mtx, VFS_MUTEX_TIMEOUT));
    vfs_ramfs_inode_t  *dirptr = dir->ramfs_file;
    vfs_ramfs_dirent_t *ent    = find_dirent(ec, vfs, dirptr, name);
    mutex_release_shared(NULL, &vfs->ramfs.mtx);

    return ent != NULL;
}



// Determine the record length for converting a RAMFS dirent to a BadgerOS dirent.
// Returns the record length for a matching `dirent_t`.
static inline size_t measure_dirent(vfs_ramfs_dirent_t *ent) {
    size_t ent_size  = offsetof(dirent_t, name) + ent->name_len + 1;
    ent_size        += (~ent_size + 1) % sizeof(size_t);
    return ent_size;
}

// Convert a RAMFS dirent to a BadgerOS dirent.
// Returns the record length for a matching `dirent_t`.
static inline size_t convert_dirent(vfs_t *vfs, dirent_t *out, vfs_ramfs_dirent_t *in) {
    vfs_ramfs_inode_t *iptr = &vfs->ramfs.inode_list[in->inode];

    out->record_len  = offsetof(dirent_t, name) + in->name_len + 1;
    out->record_len += (fileoff_t)((size_t)(~out->record_len + 1) % sizeof(size_t));
    out->inode       = in->inode;
    out->is_dir      = (iptr->mode & VFS_RAMFS_MODE_MASK) == FILETYPE_DIR << VFS_RAMFS_MODE_BIT;
    out->is_symlink  = (iptr->mode & VFS_RAMFS_MODE_MASK) == FILETYPE_LINK << VFS_RAMFS_MODE_BIT;
    out->name_len    = (fileoff_t)in->name_len;
    mem_copy(out->name, in->name, in->name_len + 1);

    return out->record_len;
}

// Atomically read all directory entries and cache them into the directory handle.
// Refer to `dirent_t` for the structure of the cache.
void vfs_ramfs_dir_read(badge_err_t *ec, vfs_t *vfs, vfs_file_handle_t *dir) {
    assert_always(mutex_acquire_shared(NULL, &vfs->ramfs.mtx, VFS_MUTEX_TIMEOUT));
    size_t             off  = 0;
    vfs_ramfs_inode_t *iptr = dir->shared->ramfs_file;

    // Measure required memory.
    size_t cap = 0;
    while (off < iptr->len) {
        vfs_ramfs_dirent_t *ent  = (vfs_ramfs_dirent_t *)(iptr->buf + off);
        cap                     += measure_dirent(ent);
        off                     += ent->size;
    }

    // Allocate memory.
    void *mem = realloc(dir->dir_cache, cap); // NOLINT
    if (!mem) {
        mutex_release_shared(NULL, &vfs->ramfs.mtx);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_NOMEM);
        return;
    }
    dir->dir_cache      = mem;
    dir->dir_cache_size = (fileoff_t)cap;

    // Generate entries.
    size_t out_off = 0;
    off            = 0;
    while (off < iptr->len) {
        vfs_ramfs_dirent_t *in  = (vfs_ramfs_dirent_t *)(iptr->buf + off);
        dirent_t           *out = (dirent_t *)(dir->dir_cache + out_off);
        convert_dirent(vfs, out, in);
        off     += in->size;
        out_off += out->record_len;
    }

    mutex_release_shared(NULL, &vfs->ramfs.mtx);
    badge_err_set_ok(ec);
}

// Atomically read the directory entry with the matching name.
// Returns true if the entry was found.
bool vfs_ramfs_dir_find_ent(badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *dir, dirent_t *out, char const *name) {
    vfs_ramfs_inode_t  *iptr = dir->ramfs_file;
    vfs_ramfs_dirent_t *in   = find_dirent(ec, vfs, iptr, name);
    if (!in) {
        return false;
    } else {
        convert_dirent(vfs, out, in);
        return true;
    }
}



// Open a file handle for the root directory.
void vfs_ramfs_root_open(badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *file) {
    assert_always(mutex_acquire_shared(NULL, &vfs->ramfs.mtx, VFS_MUTEX_TIMEOUT));

    // Install in shared file handle.
    vfs_ramfs_inode_t *iptr = &vfs->ramfs.inode_list[VFS_RAMFS_INODE_ROOT];
    file->ramfs_file        = iptr;
    file->inode             = VFS_RAMFS_INODE_ROOT;
    file->vfs               = vfs;
    file->refcount          = 1;

    iptr->links++;

    mutex_release_shared(NULL, &vfs->ramfs.mtx);
    badge_err_set_ok(ec);
}

// Open a file for reading and/or writing.
void vfs_ramfs_file_open(
    badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *dir, vfs_file_shared_t *file, char const *name
) {
    assert_always(mutex_acquire(NULL, &vfs->ramfs.mtx, VFS_MUTEX_TIMEOUT));

    // Look up the file in question.
    vfs_ramfs_inode_t  *dirptr = dir->ramfs_file;
    vfs_ramfs_dirent_t *ent    = find_dirent(ec, vfs, dirptr, name);
    if (!badge_err_is_ok(ec)) {
        mutex_release(NULL, &vfs->ramfs.mtx);
        return;
    }
    if (!ent) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_NOTFOUND);
        mutex_release(NULL, &vfs->ramfs.mtx);
        return;
    }

    // Increase refcount.
    vfs_ramfs_inode_t *iptr = &vfs->ramfs.inode_list[ent->inode];
    iptr->links++;

    // Install in shared file handle.
    file->ramfs_file = iptr;
    file->inode      = iptr->inode;
    file->vfs        = vfs;
    file->refcount   = 1;
    file->size       = (fileoff_t)iptr->len;

    mutex_release(NULL, &vfs->ramfs.mtx);
    badge_err_set_ok(ec);
}

// Close a file opened by `vfs_ramfs_file_open`.
// Only raises an error if `file` is an invalid file descriptor.
void vfs_ramfs_file_close(badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *file) {
    assert_always(mutex_acquire(NULL, &vfs->ramfs.mtx, VFS_MUTEX_TIMEOUT));
    pop_inode_refcount(vfs, file->ramfs_file);
    mutex_release(NULL, &vfs->ramfs.mtx);
    file->ramfs_file = NULL;
    badge_err_set_ok(ec);
}

// Read bytes from a file.
void vfs_ramfs_file_read(
    badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *file, fileoff_t offset, uint8_t *readbuf, fileoff_t readlen
) {
    if (offset < 0) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_RANGE);
        return;
    }
    assert_always(mutex_acquire_shared(NULL, &vfs->ramfs.mtx, VFS_MUTEX_TIMEOUT));

    vfs_ramfs_inode_t *iptr = file->ramfs_file;

    // Bounds check file and read offsets.
    if (offset + readlen > (ptrdiff_t)iptr->len || offset + readlen < offset) {
        mutex_release_shared(NULL, &vfs->ramfs.mtx);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_RANGE);
        return;
    }

    // Checks passed, return data.
    mem_copy(readbuf, iptr->buf + offset, readlen);
    mutex_release_shared(NULL, &vfs->ramfs.mtx);
    badge_err_set_ok(ec);
}

// Write bytes from a file.
void vfs_ramfs_file_write(
    badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *file, fileoff_t offset, uint8_t const *writebuf, fileoff_t writelen
) {
    if (offset < 0) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_RANGE);
        return;
    }
    assert_always(mutex_acquire(NULL, &vfs->ramfs.mtx, VFS_MUTEX_TIMEOUT));

    vfs_ramfs_inode_t *iptr = file->ramfs_file;

    // Bounds check file and read offsets.
    if (offset + writelen > (ptrdiff_t)iptr->len || offset + writelen < offset) {
        mutex_release(ec, &vfs->ramfs.mtx);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_RANGE);
        return;
    }

    // Checks passed, update data.
    mem_copy(iptr->buf + offset, writebuf, writelen);
    mutex_release(ec, &vfs->ramfs.mtx);
}

// Change the length of a file opened by `vfs_ramfs_file_open`.
void vfs_ramfs_file_resize(badge_err_t *ec, vfs_t *vfs, vfs_file_shared_t *file, fileoff_t new_size) {
    if (new_size < 0) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_RANGE);
        return;
    }
    assert_always(mutex_acquire(NULL, &vfs->ramfs.mtx, VFS_MUTEX_TIMEOUT));

    // Attempt to resize the buffer.
    vfs_ramfs_inode_t *iptr     = file->ramfs_file;
    fileoff_t          old_size = (fileoff_t)iptr->len;
    if (resize_inode(ec, vfs, iptr, new_size)) {
        file->size = new_size;
        if (new_size > old_size) {
            // Zero out new bits.
            mem_set(iptr->buf + old_size, 0, new_size - old_size);
        }
    }

    mutex_release(NULL, &vfs->ramfs.mtx);
}



// Commit all pending writes to disk.
// The filesystem, if it does caching, must always sync everything to disk at once.
void vfs_ramfs_flush(badge_err_t *ec, vfs_t *vfs) {
    // RAMFS does not do caching, so flush does nothing.
    (void)vfs;
    badge_err_set_ok(ec);
}
