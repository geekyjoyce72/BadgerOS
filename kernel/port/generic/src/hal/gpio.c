
// SPDX-License-Identifier: MIT

#include "hal/gpio.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"



// Returns the amount of GPIO pins present.
// Cannot produce an error.
int io_count() {
    return 0;
}

// Sets the mode of GPIO pin `pin` to `mode`.
void io_mode(badge_err_t *ec, int pin, io_mode_t mode) {
    badge_err_set(ec, ELOC_GPIO, ECAUSE_UNSUPPORTED);
}

// Get the mode of GPIO pin `pin`.
io_mode_t io_getmode(badge_err_t *ec, int pin) {
    badge_err_set(ec, ELOC_GPIO, ECAUSE_UNSUPPORTED);
    return IO_MODE_HIGH_Z;
}

// Sets the pull resistor behaviour of GPIO pin `pin` to `dir`.
void io_pull(badge_err_t *ec, int pin, io_pull_t dir) {
    badge_err_set(ec, ELOC_GPIO, ECAUSE_UNSUPPORTED);
}

// Get the  pull resistor behaviour of GPIO pin `pin`.
io_pull_t io_getpull(badge_err_t *ec, int pin) {
    badge_err_set(ec, ELOC_GPIO, ECAUSE_UNSUPPORTED);
    return IO_PULL_NONE;
}

// Writes level to GPIO pin pin.
void io_write(badge_err_t *ec, int pin, bool level) {
    badge_err_set(ec, ELOC_GPIO, ECAUSE_UNSUPPORTED);
}

// Reads logic level value from GPIO pin `pin`.
// Returns false on error.
bool io_read(badge_err_t *ec, int pin) {
    badge_err_set(ec, ELOC_GPIO, ECAUSE_UNSUPPORTED);
    return false;
}

// Determine whether GPIO `pin` is claimed by a peripheral.
// Returns false on error.
bool io_is_peripheral(badge_err_t *ec, int pin) {
    badge_err_set(ec, ELOC_GPIO, ECAUSE_UNSUPPORTED);
    return false;
}
