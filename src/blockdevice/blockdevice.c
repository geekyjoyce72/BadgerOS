
// SPDX-License-Identifier: MIT

#include "blockdevice.h"

#include "badge_strings.h"
#include "blockdevice/blkdev_ram.h"
#include "malloc.h"

// Prepare a block device for reading and/or writing.
// For some block devices, this may allocate caches.
void blkdev_open(badge_err_t *ec, blkdev_t *dev) {
    if (!dev) {
        badge_err_set(ec, ELOC_BLKDEV, ECAUSE_PARAM);
        return;
    }
    switch(dev->type) {
        case BLKDEV_TYPE_RAM: return blkdev_ram_open(ec, dev);
        // case BLKDEV_TYPE_I2C_EEPROM: return blkdev_i2c_eeprom_open(ec, dev);
        default: badge_err_set(ec, ELOC_BLKDEV, ECAUSE_PARAM); break;
    }
}

// Flush write caches and close block device.
void blkdev_close(badge_err_t *ec, blkdev_t *dev) {
    if (!dev) {
        badge_err_set(ec, ELOC_BLKDEV, ECAUSE_PARAM);
        return;
    }
    blkdev_flush(ec, dev);
    if (!badge_err_is_ok(ec)) return;
    switch(dev->type) {
        case BLKDEV_TYPE_RAM: return blkdev_ram_close(ec, dev);
        // case BLKDEV_TYPE_I2C_EEPROM: return blkdev_i2c_eeprom_close(ec, dev);
        default: badge_err_set(ec, ELOC_BLKDEV, ECAUSE_PARAM); break;
    }
}

// Query the erased status of a block.
// On devices which cannot erase blocks, this will always return true.
// Returns true on error.
bool blkdev_is_erased(badge_err_t *ec, blkdev_t *dev, blksize_t block) {
    if (!dev) {
        badge_err_set(ec, ELOC_BLKDEV, ECAUSE_PARAM);
        return true;
    }
    switch(dev->type) {
        case BLKDEV_TYPE_RAM: return blkdev_ram_is_erased(ec, dev, block);
        // case BLKDEV_TYPE_I2C_EEPROM: return blkdev_i2c_eeprom_is_erased(ec, dev, block);
        default: badge_err_set(ec, ELOC_BLKDEV, ECAUSE_PARAM); return true;
    }
}

// Explicitly erase a block, if possible.
// On devices which cannot erase blocks, this will do nothing.
void blkdev_erase(badge_err_t *ec, blkdev_t *dev, blksize_t block) {
    if (!dev) {
        badge_err_set(ec, ELOC_BLKDEV, ECAUSE_PARAM);
        return;
    }
    switch(dev->type) {
        case BLKDEV_TYPE_RAM: return blkdev_ram_erase(ec, dev, block);
        // case BLKDEV_TYPE_I2C_EEPROM: return blkdev_i2c_eeprom_erase(ec, dev, block);
        default: badge_err_set(ec, ELOC_BLKDEV, ECAUSE_PARAM); break;
    }
}

// Erase if necessary and write a block.
// This operation may be cached and therefor delayed.
void blkdev_write(badge_err_t *ec, blkdev_t *dev, blksize_t block, uint8_t const *writebuf) {
    if (!dev) {
        badge_err_set(ec, ELOC_BLKDEV, ECAUSE_PARAM);
        return;
    }
    switch(dev->type) {
        case BLKDEV_TYPE_RAM: return blkdev_ram_write(ec, dev, block, writebuf);
        // case BLKDEV_TYPE_I2C_EEPROM: return blkdev_i2c_eeprom_write(ec, dev, block);
        default: badge_err_set(ec, ELOC_BLKDEV, ECAUSE_PARAM); break;
    }
}

// Read a block.
// This operation may be cached.
void blkdev_read(badge_err_t *ec, blkdev_t *dev, blksize_t block, uint8_t *readbuf) {
    if (!dev) {
        badge_err_set(ec, ELOC_BLKDEV, ECAUSE_PARAM);
        return;
    }
    switch(dev->type) {
        case BLKDEV_TYPE_RAM: return blkdev_ram_read(ec, dev, block, readbuf);
        // case BLKDEV_TYPE_I2C_EEPROM: return blkdev_i2c_eeprom_read(ec, dev, block, readbuf);
        default: badge_err_set(ec, ELOC_BLKDEV, ECAUSE_PARAM); break;
    }
}

// Flush the write cache to the block device.
void blkdev_flush(badge_err_t *ec, blkdev_t *dev) {
    if (!dev) {
        badge_err_set(ec, ELOC_BLKDEV, ECAUSE_PARAM);
        return;
    }
}

// Call this function occasionally per block device to do block device housekeeping.
// Manages flushing of caches and erasure.
void blkdev_housekeeping(badge_err_t *ec, blkdev_t *dev) {
    
}

// Allocate a cache for a block device.
void blkdev_create_cache(badge_err_t *ec, blkdev_t *dev, size_t cache_depth) {

    if (!dev) {
        badge_err_set(ec, ELOC_BLKDEV, ECAUSE_PARAM);
        return;
    }
    if (dev->cache) {
        // If cache is already present, flush and remove it first.
        badge_err_t ec0;
        if (!ec) ec = &ec0;
        blkdev_flush(ec, dev);
        if (!badge_err_is_ok(ec)) return;
    }
    
    // Allocate cache info.
    dev->cache = malloc(sizeof(blkdev_cache_t));
    if (!dev->cache) {
        badge_err_set(ec, ELOC_BLKDEV, ECAUSE_NOMEM);
        return;
    }
    dev->cache->cache_depth = cache_depth;
    
    // Allocate block cache.
    dev->cache->block_cache = malloc(dev->block_size * cache_depth);
    if (!dev->cache->block_cache) {
        badge_err_set(ec, ELOC_BLKDEV, ECAUSE_NOMEM);
        free(dev->cache);
        dev->cache = NULL;
        return;
    }
    
    // Allocate block flags.
    dev->cache->block_flags = malloc(sizeof(blkdev_flags_t) * cache_depth);
    if (!dev->cache->block_flags) {
        badge_err_set(ec, ELOC_BLKDEV, ECAUSE_NOMEM);
        free(dev->cache->block_cache);
        free(dev->cache);
        dev->cache = NULL;
        return;
    }
    mem_set(dev->cache->block_flags, 0, sizeof(blkdev_flags_t) * cache_depth);
}

// Remove a cache from a block device.
void blkdev_delete_cache(badge_err_t *ec, blkdev_t *dev) {
    if (!dev) {
        badge_err_set(ec, ELOC_BLKDEV, ECAUSE_PARAM);
        return;
    }
    if (dev->cache) {
        free(dev->cache->block_cache);
        free(dev->cache->block_flags);
        free(dev->cache);
        dev->cache = NULL;
    }
}