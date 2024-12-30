
// SPDX-License-Identifier: MIT

#include "blockdevice.h"

// Clean up driver-specific parts.
typedef void (*blkdev_destroy_t)(blkdev_t *dev);
// Connect to the block device if required.
typedef void (*blkdev_open_t)(badge_err_t *ec, blkdev_t *dev);
// Disconnect from the block device.
// Writes will already have been flushed to disk.
typedef void (*blkdev_close_t)(badge_err_t *ec, blkdev_t *dev);
// Query whether a block is erased.
typedef bool (*blkdev_is_erased_t)(badge_err_t *ec, blkdev_t *dev, blksize_t block);
// Erase a block.
typedef void (*blkdev_erase_t)(badge_err_t *ec, blkdev_t *dev, blksize_t block);
// Erase if required and write a block.
typedef void (*blkdev_write_t)(badge_err_t *ec, blkdev_t *dev, blksize_t block, uint8_t const *writebuf);
// Read a block.
typedef void (*blkdev_read_t)(badge_err_t *ec, blkdev_t *dev, blksize_t block, uint8_t *readbuf);
// Partially write a block.
// This is very likely to cause a read-modify-write operation.
typedef void (*blkdev_write_partial_t)(
    badge_err_t   *ec,
    blkdev_t      *dev,
    blksize_t      block,
    size_t         subblock_offset,
    uint8_t const *writebuf,
    size_t         writebuf_len
);
// Partially read a block.
// This may use read caching if the device doesn't support partial read.
typedef void (*blkdev_read_partial_t)(
    badge_err_t *ec, blkdev_t *dev, blksize_t block, size_t subblock_offset, uint8_t *readbuf, size_t readbuf_len
);

// Block device virtual function table.
typedef struct blkdev_vtable blkdev_vtable_t;
struct blkdev_vtable {
    blkdev_destroy_t       destroy;
    blkdev_open_t          open;
    blkdev_close_t         close;
    blkdev_is_erased_t     is_erased;
    blkdev_erase_t         erase;
    blkdev_write_t         write;
    blkdev_read_t          read;
    blkdev_write_partial_t write_partial;
    blkdev_read_partial_t  read_partial;
};

// Create a new block device with a vtable and a cookie.
blkdev_t *blkdev_impl_create(badge_err_t *ec, blkdev_vtable_t const *vtable, void *cookie);
// Get a block device's cookie.
void     *blkdev_impl_get_cookie(blkdev_t const *blkdev);
// Set read-only flag.
void      blkdev_impl_set_readonly(blkdev_t *blkdev, bool readonly);
// Set block count.
void      blkdev_impl_set_size(blkdev_t *blkdev, blksize_t blocks);
// Set block size.
void      blkdev_impl_set_block_size(blkdev_t *blkdev, blksize_t block_size);
