
#pragma once

#include <stdint.h>

typedef enum {
	LOG_FATAL,
	LOG_ERROR,
	LOG_WARN,
	LOG_INFO,
	LOG_DEBUG,
} log_level_t;

// Print an unformatted message.
void logk (log_level_t level, const char *msg);
// Print a formatted message.
void logkf(log_level_t level, const char *msg, ...);
