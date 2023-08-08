
// SPDX-License-Identifier: MIT

#include "filesystem/vfs_internal.h"

#include "assertions.h"
#include "malloc.h"

#include <stdatomic.h>

// Table of mounted filesystems.
vfs_t vfs_table[FILESYSTEM_MOUNT_MAX];

// List of open shared file handles.
vfs_file_shared_t **vfs_file_shared_list;
// Number of open shared file handles.
size_t              vfs_file_shared_list_len;
// Capacity of open shared file handles list.
size_t              vfs_file_shared_list_cap;

// List of open file handles.
vfs_file_handle_t *vfs_file_handle_list;
// Number of open file handles.
size_t             vfs_file_handle_list_len;
// Capacity of open file handles list.
size_t             vfs_file_handle_list_cap;

// List of open shared directory handles.
vfs_dir_shared_t **vfs_dir_shared_list;
// Number of open shared directory handles.
size_t             vfs_dir_shared_list_len;
// Capacity of open shared directory handles list.
size_t             vfs_dir_shared_list_cap;

// List of open directory handles.
vfs_dir_handle_t *vfs_dir_handle_list;
// Number of open directory handles.
size_t            vfs_dir_handle_list_len;
// Capacity of open directory handles list.
size_t            vfs_dir_handle_list_cap;

// Next file / directory handle number.
static atomic_int vfs_handle_no = 0;



// Create a new dir_t number.
static dir_t next_dirno() {
    return atomic_fetch_add(&vfs_handle_no, 1);
}

// Create a new file_t number.
static file_t next_fileno() {
    return atomic_fetch_add(&vfs_handle_no, 1);
}



// Look up shared directory handle by pointer.
static ptrdiff_t vfs_dir_by_ptr(vfs_dir_shared_t *ptr) {
    for (size_t i = 0; i < vfs_dir_shared_list_len; i++) {
        if (vfs_dir_shared_list[i] == ptr) {
            return i;
        }
    }
    return -1;
}

// Splice a shared directory handle out of the list.
static void vfs_dir_shared_splice(ptrdiff_t i) {
    // Remove an entry.
    vfs_dir_shared_list_len--;
    if (vfs_dir_shared_list_len) {
        vfs_dir_shared_list[i] = vfs_dir_shared_list[vfs_dir_shared_list_len];
    }

    if (vfs_dir_shared_list_cap > vfs_dir_shared_list_len * 2) {
        // Shrink the array.
        size_t new_cap = vfs_dir_shared_list_cap / 2;
        if (new_cap < 2)
            new_cap = 2;
        void *mem = realloc(vfs_dir_shared_list, sizeof(*vfs_dir_shared_list) * new_cap);
        if (!mem)
            return;
        vfs_dir_shared_list     = mem;
        vfs_dir_shared_list_cap = new_cap;
    }
}

// Splice a directory handle out of the list.
static void vfs_dir_handle_splice(ptrdiff_t i) {
    // Remove an entry.
    vfs_dir_handle_list_len--;
    if (vfs_dir_handle_list_len) {
        vfs_dir_handle_list[i] = vfs_dir_handle_list[vfs_dir_handle_list_len];
    }

    if (vfs_dir_handle_list_cap > vfs_dir_handle_list_len * 2) {
        // Shrink the array.
        size_t new_cap = vfs_dir_handle_list_cap / 2;
        if (new_cap < 2)
            new_cap = 2;
        void *mem = realloc(vfs_dir_handle_list, sizeof(*vfs_dir_handle_list) * new_cap);
        if (!mem)
            return;
        vfs_dir_handle_list     = mem;
        vfs_dir_handle_list_cap = new_cap;
    }
}

// Find a shared directory handle by inode, if any.
ptrdiff_t vfs_dir_by_inode(vfs_t *vfs, inode_t inode) {
    for (size_t i = 0; i < vfs_dir_shared_list_len; i++) {
        if (vfs_dir_shared_list[i]->vfs == vfs && vfs_dir_shared_list[i]->inode == inode) {
            return i;
        }
    }
    return -1;
}

// Get a directory handle by number.
ptrdiff_t vfs_dir_by_handle(dir_t dirno) {
    for (size_t i = 0; i < vfs_dir_handle_list_len; i++) {
        if (vfs_dir_handle_list[i].dirno == dirno) {
            return i;
        }
    }
    return -1;
}

// Create a new directory handle.
// If `shared` is -1, a new empty shared handle is created.
ptrdiff_t vfs_dir_create_handle(ptrdiff_t shared) {
    if (shared == -1) {
        if (vfs_dir_shared_list_len >= vfs_dir_shared_list_cap) {
            // Expand list.
            size_t new_cap = vfs_dir_shared_list_cap * 2;
            if (new_cap < 2)
                new_cap = 2;
            void *mem = realloc(vfs_dir_shared_list, sizeof(*vfs_dir_shared_list) * new_cap);
            if (!mem)
                return -1;
            vfs_dir_shared_list     = mem;
            vfs_dir_shared_list_cap = new_cap;
        }

        // Allocate new shared handle.
        shared                  = vfs_dir_shared_list_len;
        vfs_dir_shared_t *shptr = malloc(sizeof(vfs_dir_shared_t));
        if (!shptr)
            return -1;
        *shptr = (vfs_dir_shared_t){
            .refcount = 0,
            .size     = 0,
            .inode    = 0,
            .vfs      = NULL,
        };
        vfs_dir_shared_list_len++;
    } else if (shared < -1) {
        return -1;
    }

    if (vfs_dir_handle_list_len >= vfs_dir_handle_list_cap) {
        // Expand list.
        size_t new_cap = vfs_dir_handle_list_cap * 2;
        if (new_cap < 2)
            new_cap = 2;
        void *mem = realloc(vfs_dir_handle_list, sizeof(*vfs_dir_handle_list) * new_cap);
        if (!mem)
            return -1;
        vfs_dir_handle_list     = mem;
        vfs_dir_handle_list_cap = new_cap;
    }

    // Allocate new handle.
    ptrdiff_t handle = vfs_dir_handle_list_len;
    vfs_dir_handle_list_len++;
    vfs_dir_handle_list[handle] = (vfs_dir_handle_t){
        .offset = 0,
        .shared = vfs_dir_shared_list[shared],
        .dirno  = next_dirno(),
    };

    return handle;
}

// Delete a directory handle.
// If this is the last handle referring to one directory, the shared handle is closed too.
void vfs_dir_destroy_handle(ptrdiff_t handle) {
    assert_dev_drop(handle >= 0 && handle < (ptrdiff_t)vfs_dir_handle_list_len);

    // Drop refcount.
    vfs_dir_handle_list[handle].shared->refcount--;
    if (vfs_dir_handle_list[handle].shared->refcount == 0) {
        // Close shared handle.
        vfs_dir_close(NULL, vfs_dir_handle_list[handle].shared);
        ptrdiff_t shared = vfs_dir_by_ptr(vfs_dir_handle_list[handle].shared);
        vfs_dir_shared_splice(shared);
    }

    // Splice the handle out of the list.
    vfs_dir_handle_splice(handle);
}



// Look up shared file handle by pointer.
static ptrdiff_t vfs_file_by_ptr(vfs_file_shared_t *ptr) {
    for (size_t i = 0; i < vfs_file_shared_list_len; i++) {
        if (vfs_file_shared_list[i] == ptr) {
            return i;
        }
    }
    return -1;
}

// Splice a shared file handle out of the list.
static void vfs_file_shared_splice(ptrdiff_t i) {
    // Remove an entry.
    vfs_file_shared_list_len--;
    if (vfs_file_shared_list_len) {
        vfs_file_shared_list[i] = vfs_file_shared_list[vfs_file_shared_list_len];
    }

    if (vfs_file_shared_list_cap > vfs_file_shared_list_len * 2) {
        // Shrink the array.
        size_t new_cap = vfs_file_shared_list_cap / 2;
        if (new_cap < 2)
            new_cap = 2;
        void *mem = realloc(vfs_file_shared_list, sizeof(*vfs_file_shared_list) * new_cap);
        if (!mem)
            return;
        vfs_file_shared_list     = mem;
        vfs_file_shared_list_cap = new_cap;
    }
}

// Splice a file handle out of the list.
static void vfs_file_handle_splice(ptrdiff_t i) {
    // Remove an entry.
    vfs_file_handle_list_len--;
    if (vfs_file_handle_list_len) {
        vfs_file_handle_list[i] = vfs_file_handle_list[vfs_file_handle_list_len];
    }

    if (vfs_file_handle_list_cap > vfs_file_handle_list_len * 2) {
        // Shrink the array.
        size_t new_cap = vfs_file_handle_list_cap / 2;
        if (new_cap < 2)
            new_cap = 2;
        void *mem = realloc(vfs_file_handle_list, sizeof(*vfs_file_handle_list) * new_cap);
        if (!mem)
            return;
        vfs_file_handle_list     = mem;
        vfs_file_handle_list_cap = new_cap;
    }
}

// Find a shared file handle by inode, if any.
ptrdiff_t vfs_file_by_inode(vfs_t *vfs, inode_t inode) {
    for (size_t i = 0; i < vfs_file_shared_list_len; i++) {
        if (vfs_file_shared_list[i]->vfs == vfs && vfs_file_shared_list[i]->inode == inode) {
            return i;
        }
    }
    return -1;
}

// Get a file handle by number.
ptrdiff_t vfs_file_by_handle(file_t fileno) {
    for (size_t i = 0; i < vfs_file_handle_list_len; i++) {
        if (vfs_file_handle_list[i].fileno == fileno) {
            return i;
        }
    }
    return -1;
}

// Create a new file handle.
// If `shared` is -1, a new empty shared handle is created.
ptrdiff_t vfs_file_create_handle(ptrdiff_t shared) {
    if (shared == -1) {
        if (vfs_file_shared_list_len >= vfs_file_shared_list_cap) {
            // Expand list.
            size_t new_cap = vfs_file_shared_list_cap * 2;
            if (new_cap < 2)
                new_cap = 2;
            void *mem = realloc(vfs_file_shared_list, sizeof(*vfs_file_shared_list) * new_cap);
            if (!mem)
                return -1;
            vfs_file_shared_list     = mem;
            vfs_file_shared_list_cap = new_cap;
        }

        // Allocate new shared handle.
        shared                   = vfs_file_shared_list_len;
        vfs_file_shared_t *shptr = malloc(sizeof(vfs_file_shared_t));
        if (!shptr)
            return -1;
        *shptr = (vfs_file_shared_t){
            .refcount = 0,
            .size     = 0,
            .inode    = 0,
            .vfs      = NULL,
        };
        vfs_file_shared_list_len++;
    } else if (shared < -1) {
        return -1;
    }

    if (vfs_file_handle_list_len >= vfs_file_handle_list_cap) {
        // Expand list.
        size_t new_cap = vfs_file_handle_list_cap * 2;
        if (new_cap < 2)
            new_cap = 2;
        void *mem = realloc(vfs_file_handle_list, sizeof(*vfs_file_handle_list) * new_cap);
        if (!mem)
            return -1;
        vfs_file_handle_list     = mem;
        vfs_file_handle_list_cap = new_cap;
    }

    // Allocate new handle.
    ptrdiff_t handle = vfs_file_handle_list_len;
    vfs_file_handle_list_len++;
    vfs_file_handle_list[handle] = (vfs_file_handle_t){
        .offset = 0,
        .shared = vfs_file_shared_list[shared],
        .fileno = next_fileno(),
    };

    return handle;
}

// Delete a file handle.
// If this is the last handle referring to one file, the shared handle is closed too.
void vfs_file_destroy_handle(ptrdiff_t handle) {
    assert_dev_drop(handle >= 0 && handle < (ptrdiff_t)vfs_file_handle_list_len);

    // Drop refcount.
    vfs_file_handle_list[handle].shared->refcount--;
    if (vfs_file_handle_list[handle].shared->refcount == 0) {
        // Close shared handle.
        vfs_file_close(NULL, vfs_file_handle_list[handle].shared);
        ptrdiff_t shared = vfs_file_by_ptr(vfs_file_handle_list[handle].shared);
        vfs_file_shared_splice(shared);
    }

    // Splice the handle out of the list.
    vfs_file_handle_splice(handle);
}
