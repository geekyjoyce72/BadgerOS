
// SPDX-License-Identifier: MIT

#include "blockdevice/blkdev_ram.h"

#include "badge_strings.h"
#include "blockdevice/blkdev_impl.h"



static void blkdev_ram_destroy(blkdev_t *dev) {
    (void)dev;
}

static void blkdev_ram_nop(badge_err_t *ec, blkdev_t *dev) {
    (void)dev;
    badge_err_set_ok(ec);
}

static bool blkdev_ram_is_erased(badge_err_t *ec, blkdev_t *dev, blksize_t block) {
    (void)dev;
    (void)block;
    badge_err_set_ok(ec);
    return true;
}

static void blkdev_ram_erase(badge_err_t *ec, blkdev_t *dev, blksize_t block) {
    (void)dev;
    (void)block;
    badge_err_set_ok(ec);
}

static void blkdev_ram_write(badge_err_t *ec, blkdev_t *dev, blksize_t block, uint8_t const *writebuf) {
    badge_err_set_ok(ec);
    uint8_t  *ram_addr   = blkdev_impl_get_cookie(dev);
    blksize_t block_size = blkdev_get_block_size(dev);
    mem_copy(ram_addr + block * block_size, writebuf, block_size);
}

static void blkdev_ram_read(badge_err_t *ec, blkdev_t *dev, blksize_t block, uint8_t *readbuf) {
    badge_err_set_ok(ec);
    uint8_t const *ram_addr   = blkdev_impl_get_cookie(dev);
    blksize_t      block_size = blkdev_get_block_size(dev);
    mem_copy(readbuf, ram_addr + block * block_size, block_size);
}

void blkdev_ram_write_partial(
    badge_err_t   *ec,
    blkdev_t      *dev,
    blksize_t      block,
    size_t         subblock_offset,
    uint8_t const *writebuf,
    size_t         writebuf_len
) {
    badge_err_set_ok(ec);
    uint8_t  *ram_addr   = blkdev_impl_get_cookie(dev);
    blksize_t block_size = blkdev_get_block_size(dev);
    mem_copy(ram_addr + block * block_size + subblock_offset, writebuf, writebuf_len);
}

void blkdev_ram_read_partial(
    badge_err_t *ec, blkdev_t *dev, blksize_t block, size_t subblock_offset, uint8_t *readbuf, size_t readbuf_len
) {
    badge_err_set_ok(ec);
    uint8_t const *ram_addr   = blkdev_impl_get_cookie(dev);
    blksize_t      block_size = blkdev_get_block_size(dev);
    mem_copy(readbuf, ram_addr + block * block_size + subblock_offset, readbuf_len);
}


static blkdev_vtable_t const blkdev_ram_vtable = {
    .destroy       = blkdev_ram_destroy,
    .open          = blkdev_ram_nop,
    .close         = blkdev_ram_nop,
    .is_erased     = blkdev_ram_is_erased,
    .erase         = blkdev_ram_erase,
    .write         = blkdev_ram_write,
    .read          = blkdev_ram_read,
    .write_partial = blkdev_ram_write_partial,
    .read_partial  = blkdev_ram_read_partial,
};


// Create a new RAM block device.
blkdev_t *blkdev_ram_create(badge_err_t *ec, void *memory, size_t block_count, size_t block_size, bool readonly) {
    blkdev_t *handle = blkdev_impl_create(ec, &blkdev_ram_vtable, memory);
    if (!handle) {
        return NULL;
    }
    blkdev_impl_set_block_size(handle, block_size);
    blkdev_impl_set_size(handle, block_count);
    blkdev_impl_set_readonly(handle, readonly);
    return handle;
}
