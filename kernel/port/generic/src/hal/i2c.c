
// SPDX-License-Identifier: MIT

#include "hal/i2c.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"



// Returns the amount of I²C peripherals present.
// Cannot produce an error.
int i2c_count() {
    return 0;
}


// Initialises I²C peripheral i2c_num in slave mode with SDA pin `sda_pin`, SCL pin `scl_pin` and clock speed/bitrate
// bitrate. When initialised as an I²C master, the modes of the SDA and SCL pins are changed automatically. This
// function may be used again to change the settings on an initialised I²C peripheral in master mode.
void i2c_master_init(badge_err_t *ec, int i2c_num, int sda_pin, int scl_pin, int32_t bitrate) {
    badge_err_set(ec, ELOC_I2C, ECAUSE_UNSUPPORTED);
}

// De-initialises I²C peripheral i2c_num in master mode.
void i2c_master_deinit(badge_err_t *ec, int i2c_num) {
    badge_err_set(ec, ELOC_I2C, ECAUSE_UNSUPPORTED);
}

// Reads len bytes into buffer buf from I²C slave with ID slave_id.
// This function blocks until the entire transaction is completed and returns the number of acknowledged read bytes.
size_t i2c_master_read_from(badge_err_t *ec, int i2c_num, int slave_id, void *raw_ptr, size_t len) {
    badge_err_set(ec, ELOC_I2C, ECAUSE_UNSUPPORTED);
    return 0;
}

// Writes len bytes from buffer buf to I²C slave with ID slave_id.
// This function blocks until the entire transaction is completed and returns the number of acknowledged written bytes.
size_t i2c_master_write_to(badge_err_t *ec, int i2c_num, int slave_id, void const *raw_ptr, size_t len) {
    badge_err_set(ec, ELOC_I2C, ECAUSE_UNSUPPORTED);
    return 0;
}
