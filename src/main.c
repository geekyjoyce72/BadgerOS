
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include <log.h>
#include <time.h>
#include <gpio.h>
#include <interrupt.h>



// Temporary ISR context until threading is implemented.
static isr_ctx_t isr_ctx;
// Temporary ISR registers until threading is implemented.
static isr_regs_t isr_regs;

// This is the entrypoint after the stack has been set up and the init functions have been run.
// Main is not allowed to return, so declare it noreturn.
void main() __attribute__((noreturn));
void main() {
	// First thing we do is set up the timekeeping.
	// Logs always use timestamps and watchdog feeding is currently unimplemented.
	time_init();
	
	// Install interrupt and trap handlers.
	interrupt_init(&isr_ctx, &isr_regs);
	
	// Test a log message.
	logk(LOG_FATAL, "The ultimage log message test");
	logk(LOG_ERROR, "The ultimage log message test");
	logk(LOG_WARN,  "The ultimage log message test");
	logk(LOG_INFO,  "The ultimage log message test");
	logk(LOG_DEBUG, "The ultimage log message test");
	
	// Test a system call.
	logk(LOG_DEBUG, "Pre");
	asm volatile ("ecall");
	logk(LOG_DEBUG, "Post");
	
	// Test a GPIO.
	io_mode(NULL, 15, IO_MODE_OUTPUT);
	io_mode(NULL, 22, IO_MODE_INPUT);
	io_pull(NULL, 22, IO_PULL_UP);
	while (1) {
		int64_t now = time_us();
		io_write(NULL, 15, (now / 1000000) & 1 ^ io_read(NULL, 22));
	}
}
