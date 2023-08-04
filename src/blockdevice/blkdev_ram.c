
// SPDX-License-Identifier: MIT

#include "blockdevice/blkdev_ram.h"

#include "badge_strings.h"

void blkdev_ram_open(badge_err_t *ec, blkdev_t *dev) {
    (void)dev;
    badge_err_set_ok(ec);
}

void blkdev_ram_close(badge_err_t *ec, blkdev_t *dev) {
    (void)dev;
    badge_err_set_ok(ec);
}

bool blkdev_ram_is_erased(badge_err_t *ec, blkdev_t *dev, blksize_t block) {
    if (block >= dev->blocks) {
        badge_err_set(ec, ELOC_BLKDEV, ECAUSE_RANGE);
    } else {
        badge_err_set_ok(ec);
    }
    return true;
}

void blkdev_ram_erase(badge_err_t *ec, blkdev_t *dev, blksize_t block) {
    if (block >= dev->blocks) {
        badge_err_set(ec, ELOC_BLKDEV, ECAUSE_RANGE);
    } else {
        badge_err_set_ok(ec);
    }
}

void blkdev_ram_write(badge_err_t *ec, blkdev_t *dev, blksize_t block, uint8_t const *writebuf) {
    if (block >= dev->blocks) {
        badge_err_set(ec, ELOC_BLKDEV, ECAUSE_RANGE);
    } else {
        badge_err_set_ok(ec);
        mem_copy(dev->ram_addr + block * dev->block_size, writebuf, dev->block_size);
    }
}

void blkdev_ram_read(badge_err_t *ec, blkdev_t *dev, blksize_t block, uint8_t *readbuf) {
    if (block >= dev->blocks) {
        badge_err_set(ec, ELOC_BLKDEV, ECAUSE_RANGE);
    } else {
        badge_err_set_ok(ec);
        mem_copy(readbuf, dev->ram_addr + block * dev->block_size, dev->block_size);
    }
}

void blkdev_ram_flush(badge_err_t *ec, blkdev_t *dev) {
    (void)dev;
    badge_err_set_ok(ec);
}
