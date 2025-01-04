
// SPDX-License-Identifier: MIT

#pragma once

#include "blockdevice.h"

// Block device virtual function table.
typedef struct blkdev_vtable blkdev_vtable_t;

// Flags and auxiliary data used for the cache.
typedef struct {
    // Write/erase: Timestamp of most recent sync to disk.
    // Read: Timestamp of most recent access.
    // If a cached read happens on a dirty or erase entry, the timestamp is not changed.
    timestamp_us_t update_time;
    // Block index referred to.
    blksize_t      index;
    // Cache entry contains data.
    bool           present;
    // Block is marked for erasure.
    bool           erase;
    // Cache entry differs from disk.
    bool           dirty;
} blkdev_flags_t;

// Block device cache data.
typedef struct {
    // Pointer to block cache memory.
    // Must be large enough for `cache_depth` blocks.
    uint8_t        *block_cache;
    // Pointer to block flags memory.
    // Must be large enough for `cache_depth` entries.
    blkdev_flags_t *block_flags;
    // Amount of cache entries.
    size_t          cache_depth;
} blkdev_cache_t;

// Block device descriptor.
struct blkdev {
    // Virtual function table.
    blkdev_vtable_t const *vtable;
    // Physical block size.
    blksize_t              block_size;
    // Number of blocks from this device to use.
    blksize_t              blocks;
    // Is a read-only block device.
    bool                   readonly;
    // If `cache` is nonnull: enable a read cache.
    bool                   cache_read;
    // Write and optionally read cache.
    // This may be statically allocated, or created using `blkdev_create_cache`.
    blkdev_cache_t        *cache;
    // Cookie for device driver.
    void                  *cookie;
};

// Write without caching.
void blkdev_write_raw(badge_err_t *ec, blkdev_t *dev, blksize_t block, uint8_t const *writebuf);
// Read without caching.
void blkdev_read_raw(badge_err_t *ec, blkdev_t *dev, blksize_t block, uint8_t *readbuf);
// Erase without caching.
void blkdev_erase_raw(badge_err_t *ec, blkdev_t *dev, blksize_t block);

// Perform a read-modify-write operation for partial write.
// Made for block devices that don't support partial write.
void blkdev_write_partial_fallback(
    badge_err_t   *ec,
    blkdev_t      *dev,
    blksize_t      block,
    size_t         subblock_offset,
    uint8_t const *writebuf,
    size_t         writebuf_len
);
// Read a block into a single-use read cache and copy out part of it.
// Made for block devices that don't support partial read.
void blkdev_read_partial_fallback(
    badge_err_t *ec, blkdev_t *dev, blksize_t block, size_t subblock_offset, uint8_t *readbuf, size_t readbuf_len
);
