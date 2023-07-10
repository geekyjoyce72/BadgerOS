
#include <badge_strings.h>



// Compute the length of a C-string.
size_t cstr_length(char const *string) {
	char const *ptr = string;
	while (*ptr) ptr ++;
	return ptr - string;
}

// Compute the length of a C-string at most `max_length` characters long.
size_t cstr_length_upto(char const *string, size_t max_len) {
	for (size_t i = 0; i < max_len; i++) {
		if (!string[i]) return i;
	}
	return max_len;
}

// Find the first occurrance `value` in C-string `string`.
// Returns -1 when not found.
ptrdiff_t cstr_index(char const *string, char value) {
	char const *ptr = string;
	
	// Find first index.
	while (*ptr) {
		if (*ptr == value) return ptr - string;
		ptr ++;
	}
	
	return -1;
}

// Find the last occurrance `value` in C-string `string`.
// Returns -1 when not found.
ptrdiff_t cstr_last_index(char const *string, char value) {
	return cstr_last_index_upto(string, value, SIZE_MAX);
}

// Find the first occurrance `value` in C-string `string` from and including `first_index`.
// Returns -1 when not found.
ptrdiff_t cstr_index_from(char const *string, char value, size_t first_index) {
	char const *ptr = string + first_index;
	
	// Make sure string is long enough.
	while (string++ < ptr) if (!*string) return -1;
	
	// Find first index.
	while (*ptr) {
		if (*ptr == value) return ptr - string;
		ptr ++;
	}
	
	return -1;
}

// Find the last occurrance `value` in C-string `string` up to and excluding `last_index`.
// Returns -1 when not found.
ptrdiff_t cstr_last_index_upto(char const *string, char value, size_t last_index) {
	ptrdiff_t found = -1;
	for (size_t i = 0; string[i] && i < last_index; i++) {
		if (string[i] == value) found = i;
	}
	return found;
}

// Test the equality of two C-strings.
bool cstr_equals(char const *a, char const *b) {
	while (1) {
		if (*a != *b) return false;
		if (!*a) return true;
		a++, b++;
	}
}

// Test the of the first `length` characters equality of two C-strings.
bool cstr_prefix_equals(char const *a, char const *b, size_t length) {
	while (length--) {
		if (*a != *b) return false;
		if (!*a) return true;
		a++, b++;
	}
}



// Copy at most `length`-1 characters and a NULL terminator of C-string `src` into buffer `dest`.
void cstr_copy(char *dest, size_t size, char const *src) {
	while (size-- > 1) {
		if (!*src) break;
		*dest = *src;
		dest++, src++;
	}
	if (size) {
		*dest = 0;
	}
}

// Copy at most `length` bytes C-string `src` into buffer `dest`.
// WARNING: This may leave strings without NULL terminators if `dest` does not fit `src` entirely.
void cstr_copy_packed(char *dest, size_t size, char const *src) {
	while (size--) {
		*dest = *src;
		if (!*src) return;
		dest++, src++;
	}
}



// Find the first occurrance of byte `value` in memory `memory`.
// Returns -1 when not found.
ptrdiff_t mem_index(void const *memory, size_t size, uint8_t value) {
	uint8_t *const ptr = memory;
	for (size_t i = 0; i < size; i++) {
		if (ptr[i] == value) return i;
	}
	return -1;
}

// Find the first occurrance of byte `value` in memory `memory`.
// Returns -1 when not found.
ptrdiff_t mem_last_index(void const *memory, size_t size, uint8_t value) {
	uint8_t *const ptr = memory;
	for (size_t i = size; i-- > 0;) {
		if (ptr[i] == value) return i;
	}
	return -1;
}

// Implementation of the mem_equals loop with variable read size.
#define MEM_EQUALS_IMPL(type, alignment, a, b, size) { \
		type *const a_ptr = a; \
		type *const b_ptr = b; \
		size_t _size = size / alignment; \
		for (size_t i = 0; i < _size; i++) { \
			if (a_ptr[i] != b_ptr[i]) return false; \
		} \
	}

// Test the equality of two memory areas.
bool mem_equals(void const *a, void const *b, size_t size) {
	uint_fast8_t align_detector = (uint_fast8_t) a | (uint_fast8_t) b | (uint_fast8_t) size;
	
	// Optimise for alignment.
	if (align_detector & 1) {
		MEM_EQUALS_IMPL(uint8_t, 1, a, b, size)
	} else if (align_detector & 2) {
		MEM_EQUALS_IMPL(uint16_t, 2, a, b, size)
	} else if (align_detector & 4) {
		MEM_EQUALS_IMPL(uint32_t, 4, a, b, size)
	} else {
		MEM_EQUALS_IMPL(uint64_t, 8, a, b, size)
	}
	
	return true;
}



// Implementation of the mem_copy loop with variable access size.
#define MEM_COPY_IMPL(type, alignment, dest, src, size) { \
		type *const dest_ptr = dest; \
		type *const src_ptr  = src; \
		size_t _size = size / alignment; \
		if (dest < src) { \
			/* Forward iteration. */ \
			for (size_t i = 0; i < _size; i++) { \
				dest_ptr[i] = src_ptr[i]; \
			} \
		} else if (src > dest) { \
			/* Reverse iteration. */ \
			for (size_t i = _size; i-- > 0;) { \
				dest_ptr[i] = src_ptr[i]; \
			} \
		} \
	}

// Copy the contents of memory area `src` to memory area `dest`.
// Correct copying is gauranteed even if `src` and `dest` are overlapping regions.
void mem_copy(void *dest, void const *src, size_t size) {
	uint_fast8_t align_detector = (uint_fast8_t) dest | (uint_fast8_t) src | (uint_fast8_t) size;
	
	// Optimise for alignment.
	if (align_detector & 1) {
		MEM_COPY_IMPL(uint8_t, 1, dest, src, size)
	} else if (align_detector & 2) {
		MEM_COPY_IMPL(uint16_t, 2, dest, src, size)
	} else if (align_detector & 4) {
		MEM_COPY_IMPL(uint32_t, 4, dest, src, size)
	} else {
		MEM_COPY_IMPL(uint64_t, 8, dest, src, size)
	}
}

