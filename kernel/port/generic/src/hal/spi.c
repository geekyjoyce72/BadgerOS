
// SPDX-License-Identifier: MIT

#include "hal/spi.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"



int spi_count() {
    return 0;
}

void spi_controller_init(
    badge_err_t *ec, int spi_num, int sclk_pin, int mosi_pin, int miso_pin, int ss_pin, int32_t bitrate
) {
    badge_err_set(ec, ELOC_SPI, ECAUSE_UNSUPPORTED);
}

void spi_controller_read(badge_err_t *ec, int spi_num, void *buf, size_t len) {
    badge_err_set(ec, ELOC_SPI, ECAUSE_UNSUPPORTED);
}

void spi_controller_write(badge_err_t *ec, int spi_num, void const *buf, size_t len) {
    badge_err_set(ec, ELOC_SPI, ECAUSE_UNSUPPORTED);
}

void spi_controller_transfer(badge_err_t *ec, int spi_num, void *buf, size_t len, bool fdx) {
    badge_err_set(ec, ELOC_SPI, ECAUSE_UNSUPPORTED);
}
