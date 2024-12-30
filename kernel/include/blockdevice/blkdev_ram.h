
// SPDX-License-Identifier: MIT

#include "blockdevice.h"

// Create a new RAM block device.
blkdev_t *blkdev_ram_create(badge_err_t *ec, void *memory, size_t block_count, size_t block_size, bool readonly);
