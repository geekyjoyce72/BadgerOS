
#pragma once

#include <stdint.h>

// Simple printer.
void rawprint(const char *msg);
// Simple printer.
void rawputc(char msg);
// Bin 2 hex printer.
void rawprinthex(uint64_t val, int digits);
// Bin 2 dec printer.
void rawprintudec(uint64_t val, int digits);
// Bin 2 dec printer.
void rawprintdec(int64_t val, int digits);
// Current uptime printer for logging.
void rawprintuptime();
