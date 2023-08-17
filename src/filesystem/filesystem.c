
// SPDX-License-Identifier: MIT

#include "badge_strings.h"
#include "filesystem/vfs_fat.h"
#include "filesystem/vfs_internal.h"
#include "log.h"
#include "malloc.h"



// Check the validity of a mount point.
// Creates a copy of the mount point if it is valid.
static char *check_mountpoint(badge_err_t *ec, char const *raw) {
    // Mount points are absolute paths.
    if (raw[0] != '/') {
        logkf(LOG_ERROR, "check_mountpoint: %{cs}: Mount point is relative path", raw);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_PARAM);
        return NULL;
    }

    // Check path length.
    size_t raw_len = cstr_length(raw);
    if (raw_len > FILESYSTEM_PATH_MAX) {
        logkf(LOG_ERROR, "check_mountpoint: %{cs}: Mount path too long");
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_PARAM);
        return NULL;
    }

    // Assert that the path is a canonical path.
    if (!fs_is_canonical_path(raw)) {
        logkf(LOG_ERROR, "check_mountpoint: %{cs}: Mount point is not a canonical path", raw);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_PARAM);
        return NULL;
    }

    // Create a copy of the path.
    char *copy = malloc(raw_len + 1);
    if (!copy) {
        logkf(LOG_ERROR, "check_mountpoint: Out of memory (allocating %{size;d} bytes)", raw_len + 1);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_NOMEM);
        return NULL;
    }
    mem_copy(copy, raw, raw_len + 1);
    if (raw_len > 1 && copy[raw_len] == '/') {
        copy[raw_len] = 0;
    }

    // Assert that the path is not in use.
    for (size_t i = 0; i < FILESYSTEM_MOUNT_MAX; i++) {
        if (vfs_table[i].mountpoint && cstr_equals(vfs_table[i].mountpoint, copy)) {
            logkf(LOG_ERROR, "check_mountpoint: %{cs}: Mount point is in use", copy);
            badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_INUSE);
            free(copy);
            return NULL;
        } else if (vfs_table[i].mountpoint && cstr_equals_case(vfs_table[i].mountpoint, copy)) {
            logkf(LOG_WARN, "check_mountpoint: %{cs}: Very similar to %{cs}", copy, vfs_table[i].mountpoint);
        }
    }

    // If not root filesystem, assert directory exists.
    if (!cstr_equals(copy, "/")) {
        dir_t dir = fs_dir_open(ec, copy);
        if (ec->cause == ECAUSE_NOTFOUND) {
            logkf(LOG_ERROR, "check_mountpoint: %{cs}: Mount point does not exist");
        } else if (ec->cause == ECAUSE_IS_FILE) {
            logkf(LOG_ERROR, "check_mountpoint: %{cs}: Mount point is not a directory");
        }
        if (dir == DIR_NONE) {
            free(copy);
            return NULL;
        }
        fs_dir_close(NULL, dir);
    }

    // If all that passes, the mount point is valid.
    return copy;
}

// Try to mount a filesystem.
void fs_mount(badge_err_t *ec, fs_type_t type, blkdev_t *media, char const *mountpoint, mountflags_t flags) {
    badge_err_t ec0;
    if (!ec)
        ec = &ec0;

    // Take the filesystem mounting mutex.
    if (!mutex_acquire(NULL, &vfs_mount_mtx, VFS_MUTEX_TIMEOUT)) {
        logk(LOG_ERROR, "fs_mount: Timeout while acquiring mutex.");
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_TIMEOUT);
        return;
    }

    if (type == FS_TYPE_AUTO) {
        // Try to auto-detect filesystem.
        type = fs_detect(ec, media);
        if (!badge_err_is_ok(ec))
            return;
        if (type == FS_TYPE_AUTO) {
            logk(LOG_ERROR, "fs_mount: Unable to determine filesystem type.");
            badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_UNAVAIL);
            return;
        }

    } else {
        // Confirm filesystem type.
        bool res;
        switch (type) {
            case FS_TYPE_FAT: res = vfs_fat_detect(ec, media); break;
            default: badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_PARAM); return;
        }
        if (!res) {
            logk(LOG_ERROR, "fs_mount: Requested filesystem not detected.");
            badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_UNAVAIL);
        }
    }

    // Check the validity of the mount point.
    char *mountpoint_copy = check_mountpoint(ec, mountpoint);
    if (!badge_err_is_ok(ec)) {
        mutex_release(NULL, &vfs_mount_mtx);
        return;
    }

    // Check for space in VFS table.
    size_t vfs_index;
    for (vfs_index = 0; vfs_index < FILESYSTEM_MOUNT_MAX; vfs_index++) {
        if (!vfs_table[vfs_index].mountpoint) {
            break;
        }
    }
    if (vfs_index == FILESYSTEM_MOUNT_MAX) {
        logk(LOG_ERROR, "fs_mount: Mounted filesystem limit (" comptime_stringify(FILESYSTEM_MOUNT_MAX) ") reached.");
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_UNAVAIL);
        free(mountpoint_copy);
        mutex_release(NULL, &vfs_mount_mtx);
        return;
    }

    // Check writeability.
    if (!(flags & MOUNTFLAGS_READONLY) && media->readonly) {
        logk(LOG_ERROR, "fs_mount: Writeable filesystem on readonly media.");
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_READONLY);
        goto error_cleanup;
    }

    // Fill out the VFS entry.
    vfs_table[vfs_index] = (vfs_t){
        .mountpoint = mountpoint_copy,
        .readonly   = flags & MOUNTFLAGS_READONLY,
        .media      = media,
        .type       = type,
    };

    // Delegate to filesystem-specific mount.
    switch (type) {
        case FS_TYPE_FAT: vfs_fat_mount(ec, &vfs_table[vfs_index]); break;
        default: __builtin_unreachable();
    }
    if (!badge_err_is_ok(ec)) {
        logk(LOG_ERROR, "fs_mount: Mount error reported by VFS.");
        goto error_cleanup;
    }

    // At this point, the filesystem is ready for use.
    mutex_release(NULL, &vfs_mount_mtx);
    return;

error_cleanup:
    free(vfs_table[vfs_index].mountpoint);
    vfs_table[vfs_index].mountpoint = NULL;
    mutex_release(NULL, &vfs_mount_mtx);
}

// Unmount a filesystem.
// Only raises an error if there isn't a valid filesystem to unmount.
void fs_umount(badge_err_t *ec, char const *mountpoint) {
    // TODO: Remove redundant slashes from path.
    size_t vfs_index;
    for (vfs_index = 0; vfs_index < FILESYSTEM_MOUNT_MAX; vfs_index++) {
        if (cstr_equals(vfs_table[vfs_index].mountpoint, mountpoint)) {
            break;
        }
    }
    if (vfs_index == FILESYSTEM_MOUNT_MAX) {
        logkf(LOG_ERROR, "fs_umount: %{cs}: Not mounted.", mountpoint);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_NOTFOUND);
        return;
    }

    // Close file handles.
    for (size_t i = 0; i < vfs_dir_handle_list_len; i++) {
        if (vfs_dir_handle_list[i].shared->vfs == &vfs_table[vfs_index]) {
            fs_dir_close(NULL, vfs_dir_handle_list[i].dirno);
        }
    }
    for (size_t i = 0; i < vfs_file_handle_list_len; i++) {
        if (vfs_file_handle_list[i].shared->vfs == &vfs_table[vfs_index]) {
            fs_close(NULL, vfs_file_handle_list[i].fileno);
        }
    }

    // Delegate to filesystem-specific mount.
    switch (vfs_table[vfs_index].type) {
        case FS_TYPE_FAT: vfs_fat_umount(&vfs_table[vfs_index]); break;
        default: __builtin_unreachable();
    }

    // Release memory.
    free(vfs_table[vfs_index].mountpoint);
    vfs_table[vfs_index].mountpoint = NULL;
}

// Try to identify the filesystem stored in the block device
// Returns `FS_TYPE_AUTO` on error or if the filesystem is unknown.
fs_type_t fs_detect(badge_err_t *ec, blkdev_t *media) {
    if (vfs_fat_detect(ec, media)) {
        return FS_TYPE_FAT;
    } else {
        badge_err_set_ok(ec);
        return FS_TYPE_AUTO;
    }
}



// Get the canonical path of a file or directory.
// Allocates a new c-string to do so.
char *fs_to_canonical_path(badge_err_t *ec, char const *path) {
    badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_UNSUPPORTED);
    return NULL;
}

// Test whether a path is a canonical path, but not for the existence of the file or directory.
// A canonical path starts with '/' and contains none of the following regex: `\.\.?/|\.\.?$|//+`
bool fs_is_canonical_path(char const *path) {
    if (*path != '/') {
        return false;
    }
    while (*path) {
        if (path[0] == '.') {
            size_t i = path[1] == '.';
            if (path[i] == 0 || path[i] == '/') {
                return false;
            }
        } else if (path[0] == '/' && path[1] == '/') {
            return false;
        }
        ptrdiff_t index = cstr_index(path, '/');
        if (index == -1) {
            return true;
        }
        path = path + index + 1;
    }
}



// Open a directory for reading.
dir_t fs_dir_open(badge_err_t *ec, char const *path) {
    badge_err_t ec0 = {0, 0};
    if (!ec)
        ec = &ec0;

    // Read the directory entry.
    bool              found  = false;
    vfs_dir_handle_t *parent = vfs_walk(ec, path, &found, true);
    if (!badge_err_is_ok(ec))
        return DIR_NONE;
    if (!found) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_NOTFOUND);
        return DIR_NONE;
    }

    ptrdiff_t existing;
    if (parent) {
        // Path is not root directory.
        dirent_t ent = vfs_dir_read_ent(ec, parent->shared, parent->offset);
        if (!badge_err_is_ok(ec))
            goto error;

        // Check file type.
        if (!ent.is_dir) {
            badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_IS_FILE);
            goto error;
        }

        // Check for existing shared handles.
        while (!mutex_acquire(NULL, &vfs_handle_mtx, VFS_MUTEX_TIMEOUT))
            ;
        existing = vfs_dir_by_inode(parent->shared->vfs, ent.inode);
    } else {
        // Path is root directory.
        // Check for existing shared handles.
        while (!mutex_acquire(NULL, &vfs_handle_mtx, VFS_MUTEX_TIMEOUT))
            ;
        existing = vfs_dir_by_inode(&vfs_table[vfs_root_index], INODE_ROOT);
    }

    // Create a new handle from existing shared handle.
    ptrdiff_t handle = vfs_dir_create_handle(existing);
    if (handle == -1) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_NOMEM);
        mutex_release(NULL, &vfs_handle_mtx);
        goto error;
    }
    vfs_dir_handle_t *ptr = &vfs_dir_handle_list[handle];

    if (existing == -1) {
        // Create new shared directory handle.
        vfs_dir_shared_t *shared = ptr->shared;
        vfs_dir_open(ec, parent, shared);
        if (badge_err_is_ok(ec)) {
            vfs_dir_destroy_handle(handle);
            mutex_release(NULL, &vfs_handle_mtx);
            goto error;
        }
        shared->refcount = 1;
    }

    // Successful opening of new handle.
    mutex_release(NULL, &vfs_handle_mtx);
    return ptr->dirno;

error:
    // Destroy directory handle.
    while (!mutex_acquire(NULL, &vfs_handle_mtx, VFS_MUTEX_TIMEOUT))
        ;
    ptrdiff_t index = vfs_dir_by_handle(parent->dirno);
    vfs_dir_destroy_handle(index);
    mutex_release(NULL, &vfs_handle_mtx);
    return DIR_NONE;
}

// Close a directory opened by `fs_dir_open`.
// Only raises an error if `dir` is an invalid directory descriptor.
void fs_dir_close(badge_err_t *ec, dir_t dir) {
    if (!mutex_acquire(ec, &vfs_handle_mtx, VFS_MUTEX_TIMEOUT)) {
        return;
    }

    ptrdiff_t index = vfs_dir_by_handle(dir);
    if (index != -1) {
        vfs_dir_destroy_handle(index);
    }

    mutex_release(ec, &vfs_handle_mtx);
}

// Read the current directory entry (but not the filename).
// Only moves to the next entry if `next` is true.
// See also: `fs_dir_read_name`.
dirent_t fs_dir_read_ent(badge_err_t *ec, dir_t dir, bool next) {
    badge_err_t ec0 = {0, 0};
    if (!ec)
        ec = &ec0;

    if (!mutex_acquire_shared(ec, &vfs_handle_mtx, VFS_MUTEX_TIMEOUT)) {
        return (dirent_t){};
    }

    ptrdiff_t index = vfs_dir_by_handle(dir);
    if (index != -1) {
        vfs_dir_handle_t *handle = &vfs_dir_handle_list[index];
        dirent_t          ent    = vfs_dir_read_ent(ec, handle->shared, handle->offset);
        if (badge_err_is_ok(ec) && next) {
            vfs_dir_next(ec, handle->shared, &handle->offset);
        }
        mutex_release_shared(ec, &vfs_handle_mtx);
        return ent;
    } else {
        mutex_release_shared(ec, &vfs_handle_mtx);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_PARAM);
        return (dirent_t){};
    }
}

// Read the current directory entry (only the null-terminated filename).
// Returns the string length of the filename.
// Only moves to the next entry if `next` is true.
// See also: `fs_dir_read_ent`.
size_t fs_dir_read_name(badge_err_t *ec, dir_t dir, char *buf, size_t buf_len, bool next) {
    badge_err_t ec0 = {0, 0};
    if (!ec)
        ec = &ec0;

    if (!mutex_acquire_shared(ec, &vfs_handle_mtx, VFS_MUTEX_TIMEOUT)) {
        return 0;
    }

    ptrdiff_t index = vfs_dir_by_handle(dir);
    if (index != -1) {
        vfs_dir_handle_t *handle = &vfs_dir_handle_list[index];
        size_t            len    = vfs_dir_read_name(ec, handle->shared, handle->offset, buf, buf_len);
        if (badge_err_is_ok(ec) && next) {
            vfs_dir_next(ec, handle->shared, &handle->offset);
        }
        mutex_release_shared(ec, &vfs_handle_mtx);
        return len;
    } else {
        mutex_release_shared(ec, &vfs_handle_mtx);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_PARAM);
        return 0;
    }
}



// Open a file for reading and/or writing.
file_t fs_open(badge_err_t *ec, char const *path, oflags_t oflags) {
    badge_err_t ec0 = {0, 0};
    if (!ec)
        ec = &ec0;

    // Read the directory entry.
    bool              found  = false;
    vfs_dir_handle_t *parent = vfs_walk(ec, path, &found, true);
    if (!badge_err_is_ok(ec)) {
        return FILE_NONE;
    }

    // Check file exists.
    if (found && (oflags & OFLAGS_EXCLUSIVE)) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_EXISTS);
        goto error;
    } else if (!found && !(oflags & OFLAGS_CREATE) && !(oflags & OFLAGS_EXCLUSIVE)) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_NOTFOUND);
        goto error;
    }

    // If the file doesn't exist, create it.
    if (!found) {
    }

    ptrdiff_t existing;
    if (parent) {
        // Path is not root directory.
        dirent_t ent = vfs_dir_read_ent(ec, parent->shared, parent->offset);
        if (!badge_err_is_ok(ec))
            goto error;

        // Check file type.
        if (!ent.is_dir) {
            badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_IS_FILE);
            goto error;
        }

        // Check for existing shared handles.
        while (!mutex_acquire(NULL, &vfs_handle_mtx, VFS_MUTEX_TIMEOUT))
            ;
        existing = vfs_file_by_inode(parent->shared->vfs, ent.inode);
    } else {
        // Path is root directory.
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_IS_DIR);
        goto error;
    }

    // Create a new handle from existing shared handle.
    ptrdiff_t handle = vfs_file_create_handle(existing);
    if (handle == -1) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_NOMEM);
        mutex_release(NULL, &vfs_handle_mtx);
        goto error;
    }
    vfs_file_handle_t *ptr = &vfs_file_handle_list[handle];

    if (existing == -1) {
        // Create new shared directory handle.
        vfs_file_shared_t *shared = ptr->shared;
        vfs_file_open(ec, parent, shared, oflags);
        if (badge_err_is_ok(ec)) {
            vfs_file_destroy_handle(handle);
            mutex_release(NULL, &vfs_handle_mtx);
            goto error;
        }
        shared->refcount = 1;
    }

    // Successful opening of new handle.
    mutex_release(NULL, &vfs_handle_mtx);

    // Apply opening flags.
    ptr->read  = oflags & OFLAGS_READONLY;
    ptr->write = oflags & OFLAGS_WRITEONLY;

    return ptr->fileno;

error:
    // Destroy directory handle.
    while (!mutex_acquire(NULL, &vfs_handle_mtx, VFS_MUTEX_TIMEOUT))
        ;
    ptrdiff_t index = vfs_dir_by_handle(parent->dirno);
    vfs_dir_destroy_handle(index);
    mutex_release(NULL, &vfs_handle_mtx);
    return FILE_NONE;
}

// Clone a file opened by `fs_open`.
// Only raises an error if `file` is an invalid file descriptor.
void fs_close(badge_err_t *ec, file_t file) {
}

// Read bytes from a file.
// Returns the amount of data successfully read.
filesize_t fs_read(badge_err_t *ec, file_t file, uint8_t *readbuf, filesize_t readlen) {
}

// Write bytes to a file.
// Returns the amount of data successfully written.
filesize_t fs_write(badge_err_t *ec, file_t file, uint8_t const *writebuf, filesize_t writelen) {
}

// Get the current offset in the file.
fileoff_t fs_tell(badge_err_t *ec, file_t file) {
}

// Set the current offset in the file.
// Returns the new offset in the file.
fileoff_t fs_seek(badge_err_t *ec, file_t file, fileoff_t off, fs_seek_t seekmode) {
}

// Force any write caches to be flushed for a given file.
// If the file is `FILE_NONE`, all open files are flushed.
void fs_flush(badge_err_t *ec, file_t file) {
}
