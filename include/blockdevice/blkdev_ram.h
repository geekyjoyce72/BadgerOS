
// SPDX-License-Identifier: MIT

#include "blockdevice.h"

// Connect to the block device if required.
void blkdev_ram_open(badge_err_t *ec, blkdev_t *dev);
// Disconnect from the block device.
// Writes will already have been flushed to disk.
void blkdev_ram_close(badge_err_t *ec, blkdev_t *dev);
// Query whether a block is erased.
bool blkdev_ram_is_erased(badge_err_t *ec, blkdev_t *dev, blksize_t block);
// Erase a block.
void blkdev_ram_erase(badge_err_t *ec, blkdev_t *dev, blksize_t block);
// Erase if required and write a block.
void blkdev_ram_write(badge_err_t *ec, blkdev_t *dev, blksize_t block, uint8_t const *writebuf);
// Read a block.
void blkdev_ram_read(badge_err_t *ec, blkdev_t *dev, blksize_t block, uint8_t *readbuf);
// Check whether caching is recommended.
bool blkdev_ram_do_cache(badge_err_t *ec, blkdev_t *dev);