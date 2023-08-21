
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
        file_t dir = fs_dir_open(ec, copy);
        if (ec->cause == ECAUSE_NOTFOUND) {
            logkf(LOG_ERROR, "check_mountpoint: %{cs}: Mount point does not exist");
        } else if (ec->cause == ECAUSE_IS_FILE) {
            logkf(LOG_ERROR, "check_mountpoint: %{cs}: Mount point is not a directory");
        }
        if (dir == FILE_NONE) {
            free(copy);
            return NULL;
        }
        fs_dir_close(NULL, dir);
    }

    // If all that passes, the mount point is valid.
    return copy;
}

// Try to mount a filesystem.
// Some filesystems (like RAMFS) do not use a block device, for which `media` must be NULL.
// Filesystems which do use a block device can often be automatically detected.
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

    if (type == FS_TYPE_UNKNOWN && !media) {
        // Block device is required to auto-detect.
        logk(LOG_ERROR, "fs_mount: Neither media nor type specified.");
        mutex_release(NULL, &vfs_mount_mtx);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_PARAM);
    } else if (type == FS_TYPE_UNKNOWN) {
        // Try to auto-detect filesystem.
        type = fs_detect(ec, media);
        if (!badge_err_is_ok(ec))
            return;
        if (type == FS_TYPE_UNKNOWN) {
            logk(LOG_ERROR, "fs_mount: Unable to determine filesystem type.");
            mutex_release(NULL, &vfs_mount_mtx);
            badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_UNAVAIL);
            return;
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
    if (!(flags & MOUNTFLAGS_READONLY) && media && media->readonly) {
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
// Returns `FS_TYPE_UNKNOWN` on error or if the filesystem is unknown.
fs_type_t fs_detect(badge_err_t *ec, blkdev_t *media) {
    if (vfs_fat_detect(ec, media)) {
        return FS_TYPE_FAT;
    } else {
        badge_err_set_ok(ec);
        return FS_TYPE_UNKNOWN;
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



// Test that the handle exists and is a directory handle.
static bool is_dir_handle(badge_err_t *ec, file_t dir) {
    if (!mutex_acquire_shared(ec, &vfs_handle_mtx, TIMESTAMP_US_MAX)) {
        __builtin_unreachable();
    }

    // Check the handle exists.
    ptrdiff_t index = vfs_file_by_handle(dir);
    if (index < 0) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_PARAM);
        mutex_release_shared(ec, &vfs_handle_mtx);
        return false;
    }
    // Check the handle is that of a directory.
    vfs_file_handle_t *handle = &vfs_file_handle_list[index];
    if (!handle->is_dir) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_IS_FILE);
        mutex_release_shared(ec, &vfs_handle_mtx);
        return false;
    }

    mutex_release_shared(ec, &vfs_handle_mtx);
    return true;
}

// Open a directory for reading.
file_t fs_dir_open(badge_err_t *ec, char const *path) {
    return fs_open(ec, path, OFLAGS_DIRECTORY);
}

// Close a directory opened by `fs_dir_open`.
// Only raises an error if `dir` is an invalid directory descriptor.
void fs_dir_close(badge_err_t *ec, file_t dir) {
    if (!is_dir_handle(ec, dir))
        return;
    fs_close(ec, dir);
}

// Read the current directory entry.
// See also: `fs_dir_read_name`.
void fs_dir_read(badge_err_t *ec, dirent_t *dirent_out, file_t dir) {
    if (!is_dir_handle(ec, dir))
        return;
    badge_err_t ec0;
    if (!ec)
        ec = &ec0;

    // Read the entry but not the name.
    fileoff_t pos = fs_tell(ec, dir);
    if (!badge_err_is_ok(ec))
        return;
    fileoff_t read_len = sizeof(dirent_t) - FILESYSTEM_NAME_MAX - 1;
    fileoff_t len      = fs_read(ec, dir, &dirent_out, read_len);
    // Bounds check read, name and record length.
    if (!badge_err_is_of(ec) || len != read_len || dirent_out->name_len > FILESYSTEM_NAME_MAX ||
        dirent_out->record_len < read_len + dirent_out->name_len) {
        // Revert position, report error.
        fs_seek(NULL, dir, pos, SEEK_ABS);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_UNKNOWN);
        return;
    }

    // Read the name.
    fileoff_t name_len = fs_read(ec, dir, dirent_out->name, dirent_out->name_len);
    // Bounds check read, test name validity.
    if (!badge_err_is_of(ec) || name_len != dirent_out->name_len || mem_index(dirent_out->name, name_len, '/') != -1 ||
        mem_index(dirent_out->name, name_len, 0) != -1) {
        // Revert position, report error.
        fs_seek(NULL, dir, pos, SEEK_ABS);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_UNKNOWN);
        return;
    }
    // Null-terminate name.
    dirent_out->name[name_len] = 0;
}



// Open a file for reading and/or writing.
file_t fs_open(badge_err_t *ec, char const *path, oflags_t oflags) {
    badge_err_t ec0 = {0, 0};
    if (!ec)
        ec = &ec0;

    // Test flag validity.
    if ((oflags & OFLAGS_DIRECTORY) && (oflags & ~VALID_OFLAGS_DIRECTORY)) {
        // A flag mutually exclusive with `OFLAGS_DIRECTORY` was given.
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_PARAM);
        return FILE_NONE;
    } else if (oflags & ~VALID_OFLAGS_FILE) {
        // An invalid flag was given.
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_PARAM);
        return FILE_NONE;
    } else if (!(oflags & OFLAGS_READWRITE) || ((oflags & OFLAGS_DIRECTORY) && !(oflags & OFLAGS_READONLY))) {
        // Neither read nor write access is requested.
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_PARAM);
        return FILE_NONE;
    }

    // Copy the path.
    char canon_path[FILESYSTEM_PATH_MAX + 1];
    if (path[cstr_copy(canon_path, sizeof(canon_path), path)] != 0) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_TOOLONG);
        return FILE_NONE;
    }

    // Locate the file.
    bool               found  = false;
    vfs_file_handle_t *parent = vfs_walk(ec, canon_path, &found, true);
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
        // Get the filename from canonical path to create the file.
        char     *filename;
        ptrdiff_t slash = cstr_index(canon_path, '/');
        if (slash == -1) {
            filename = canon_path;
        } else {
            filename = canon_path + slash + 1;
        }
        vfs_create_file(ec, parent, filename);
    }

    ptrdiff_t existing = -1;
    bool      is_dir;
    if (parent) {
        // Path is not root directory.
        dirent_t ent;
        fs_dir_read(ec, &ent, parent->fileno);
        if (!badge_err_is_ok(ec))
            goto error;
        is_dir = ent.is_dir;

        // Check file type.
        if (ent.is_dir != !!(oflags & OFLAGS_DIRECTORY)) {
            badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_IS_FILE);
            goto error;
        }

        // Check for existing shared handles.
        if (!mutex_acquire_shared(ec, &vfs_handle_mtx, TIMESTAMP_US_MAX)) {
            __builtin_unreachable();
        }
        existing = vfs_file_by_inode(parent->shared->vfs, ent.inode);
    } else {
        // Path is root directory.
        if (!(oflags & OFLAGS_DIRECTORY)) {
            badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_IS_DIR);
            goto error;
        }
        is_dir   = true;
        existing = vfs_file_by_inode(&vfs_table[vfs_root_index], INODE_ROOT);
    }

    // Create a new handle from existing shared handle.
    ptrdiff_t handle = vfs_file_create_handle(existing);
    if (handle == -1) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_NOMEM);
        mutex_release(NULL, &vfs_handle_mtx);
        goto error;
    }
    vfs_file_handle_t *ptr = &vfs_file_handle_list[handle];

    // Apply opening flags.
    ptr->read   = oflags & OFLAGS_READONLY;
    ptr->write  = oflags & OFLAGS_WRITEONLY;
    ptr->is_dir = is_dir;

    if (existing == -1) {
        // Create new shared file handle.
        vfs_file_shared_t *shared = ptr->shared;
        vfs_file_open(ec, parent, shared, oflags);
        if (badge_err_is_ok(ec)) {
            vfs_file_destroy_handle(handle);
            mutex_release(NULL, &vfs_handle_mtx);
            goto error;
        }
        shared->refcount = 1;
    }
    if (is_dir) {
        // Cache all the directory entries.
        vfs_dir_read(ec, ptr);
    }

    // Successful opening of new handle.
    mutex_release(NULL, &vfs_handle_mtx);

    return ptr->fileno;

error:
    // Destroy directory handle.
    if (!mutex_acquire_shared(ec, &vfs_handle_mtx, TIMESTAMP_US_MAX)) {
        __builtin_unreachable();
    }
    ptrdiff_t index = vfs_dir_by_handle(parent->fileno);
    vfs_file_destroy_handle(index);
    mutex_release(NULL, &vfs_handle_mtx);
    return FILE_NONE;
}

// Close a file opened by `fs_open`.
// Only raises an error if `file` is an invalid file descriptor.
void fs_close(badge_err_t *ec, file_t file) {
    if (!mutex_acquire_shared(ec, &vfs_handle_mtx, TIMESTAMP_US_MAX)) {
        __builtin_unreachable();
    }

    ptrdiff_t index = vfs_file_by_handle(file);
    if (index == -1) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_PARAM);
    } else {
        badge_err_set_ok(ec);
        vfs_file_destroy_handle(index);
    }

    mutex_release(NULL, &vfs_handle_mtx);
}

// Read bytes from a file.
// Returns the amount of data successfully read.
fileoff_t fs_read(badge_err_t *ec, file_t file, uint8_t *readbuf, fileoff_t readlen) {
    if (readlen < 0) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_PARAM);
        mutex_release_shared(NULL, &vfs_handle_mtx);
        return 0;
    }
    if (!mutex_acquire_shared(NULL, &vfs_handle_mtx, VFS_MUTEX_TIMEOUT)) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_TIMEOUT);
        return 0;
    }

    // Look up the handle.
    ptrdiff_t index = vfs_file_by_handle(file);
    if (index == -1) {
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_PARAM);
        mutex_release_shared(NULL, &vfs_handle_mtx);
        return 0;
    }
    vfs_file_handle_t *ptr = &vfs_file_handle_list[index];

    // Read data from the handle.
    if (ptr->is_dir) {
        if (readlen + ptr->offset < 0 || readlen + ptr->offset > ptr->dir_cache_size) {
            readlen = ptr->dir_cache_size - ptr->offset;
        }
        mem_copy(readbuf, ptr->dir_cache + ptr->offset, readlen);
        ptr->offset += readlen;
    } else {
        if (readlen + ptr->offset < 0 || readlen + ptr->offset > ptr->shared->size) {
            readlen = ptr->shared->size - ptr->offset;
        }
        vfs_file_read(ec, ptr->shared, ptr->offset, readbuf, readlen);
    }

    mutex_release_shared(NULL, &vfs_handle_mtx);
}

// Write bytes to a file.
// Returns the amount of data successfully written.
fileoff_t fs_write(badge_err_t *ec, file_t file, uint8_t const *writebuf, fileoff_t writelen) {
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
