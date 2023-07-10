
// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>



// Compute the length of a C-string.
size_t    cstr_length         (char const *string) __attribute__((pure));
// Compute the length of a C-string at most `max_length` characters long.
size_t    cstr_length_upto    (char const *string, size_t max_len) __attribute__((pure));
// Find the first occurrance `value` in C-string `string`.
// Returns -1 when not found.
ptrdiff_t cstr_index          (char const *string, char value) __attribute__((pure));
// Find the last occurrance `value` in C-string `string`.
// Returns -1 when not found.
ptrdiff_t cstr_last_index     (char const *string, char value) __attribute__((pure));
// Find the first occurrance `value` in C-string `string` from and including `first_index`.
// Returns -1 when not found.
ptrdiff_t cstr_index_from     (char const *string, char value, size_t first_index) __attribute__((pure));
// Find the last occurrance `value` in C-string `string` up to and excluding `last_index`.
// Returns -1 when not found.
ptrdiff_t cstr_last_index_upto(char const *string, char value, size_t last_index) __attribute__((pure));
// Test the equality of two C-strings.
bool      cstr_equals         (char const *a, char const *b) __attribute__((pure));
// Test the of the first `length` characters equality of two C-strings.
bool      cstr_prefix_equals  (char const *a, char const *b, size_t length) __attribute__((pure));

// Copy at most `length`-1 characters and a NULL terminator of C-string `src` into buffer `dest`.
void cstr_copy(char *dest, size_t size, char const *src);
// Copy at most `length` bytes C-string `src` into buffer `dest`.
// WARNING: This may leave strings without NULL terminators if `dest` does not fit `src` entirely.
void cstr_copy_packed(char *dest, size_t size, char const *src);

// Find the first occurrance of byte `value` in memory `memory`.
// Returns -1 when not found.
ptrdiff_t mem_index     (void const *memory, size_t size, uint8_t value) __attribute__((pure));
// Find the first occurrance of byte `value` in memory `memory`.
// Returns -1 when not found.
ptrdiff_t mem_last_index(void const *memory, size_t size, uint8_t value) __attribute__((pure));
// Test the equality of two memory areas.
bool      mem_equals    (void const *a, void const *b, size_t size) __attribute__((pure));

// Copy the contents of memory area `src` to memory area `dest`.
// Correct copying is gauranteed even if `src` and `dest` are overlapping regions.
void mem_copy(void *dest, void const *src, size_t size);
