
// SPDX-License-Identifier: MIT

#include "badge_strings.h"
#include "filesystem/vfs_fat.h"
#include "filesystem/vfs_internal.h"
#include "log.h"
#include "malloc.h"

// Try to mount a filesystem.
void fs_mount(badge_err_t *ec, fs_type_t type, blkdev_t *media, char const *mountpoint, mountflags_t flags) {
    badge_err_t ec0;
    if (!ec)
        ec = &ec0;

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

    // Bounds check.
    size_t mountpoint_len = cstr_length_upto(mountpoint, FILESYSTEM_PATH_MAX + 1);
    if (mountpoint_len > FILESYSTEM_PATH_MAX) {
        logk(LOG_ERROR, "fs_mount: Mount path too long.");
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_TOOLONG);
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
        return;
    }

    // Check writeability.
    if (!(flags & MOUNTFLAGS_READONLY) && media->readonly) {
        logk(LOG_ERROR, "fs_mount: Writeable filesystem on readonly media.");
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_READONLY);
        return;
    }

    // TODO: Enforce an empty directory exists at mount point.
    // TODO: Enforce name constraints.
    // TODO: Remove redundant slashes from path.

    // Fill out the VFS entry.
    vfs_table[vfs_index] = (vfs_t){
        .mountpoint = malloc(mountpoint_len + 1),
        .readonly   = flags & MOUNTFLAGS_READONLY,
        .media      = media,
        .type       = type,
    };
    if (!vfs_table[vfs_index].mountpoint) {
        logkf(LOG_ERROR, "fs_mount: Out of memory (allocating %{size;d} bytes).", mountpoint_len + 1);
        badge_err_set(ec, ELOC_FILESYSTEM, ECAUSE_NOMEM);
        return;
    }
    mem_copy(vfs_table[vfs_index].mountpoint, mountpoint, mountpoint_len + 1);

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
    return;

error_cleanup:
    free(vfs_table[vfs_index].mountpoint);
    vfs_table[vfs_index].mountpoint = NULL;
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
        return FS_TYPE_AUTO;
    }
}
