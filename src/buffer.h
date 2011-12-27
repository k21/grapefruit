#ifndef GRAPEFRUIT_BUFFER_H_
#define GRAPEFRUIT_BUFFER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "common.h"
#include "list.h"

struct buffer_chunk {
	uintptr_t full;
	char* data;
};

// buffer provides buffered input and allows to output last line if it matches
struct buffer {
	int fd_in, fd_out;
	uintptr_t max_chunk_size;
	intptr_t pos;
	intptr_t mark;
	struct list chunks;
	struct buffer_chunk current;
};

// Allocate and initialize new buffer
struct buffer* new_buffer(int in, int out, uintptr_t size);

// Initialize buffer
void buffer_init(struct buffer* buffer, int in, int out, uintptr_t size);

// Initialize buffer chunk (stores one continuous chunk of input data)
static void buffer_chunk_init(struct buffer_chunk* chunk, uintptr_t size);

// Read next byte from input
// On success returns 1, on error -1, on EOF 0
static inline int_fast8_t buffer_next(struct buffer* buffer);

// Return the last read byte
static inline inline uint_fast8_t buffer_get(struct buffer* buffer);

// Remove last mark and mark current position in input
void buffer_mark(struct buffer* buffer);

// Print buffer contents from last mark to previous read byte and remove mark
// If print_current is true, also print the last read byte
// Returns 1 on success, -1 on error, 0 when only part of data was written
int_fast8_t buffer_print(struct buffer* buffer, bool print_current);

// Clean up buffer_chunk, remove all structs it used
static void buffer_chunk_cleanup(struct buffer_chunk* chunk);

// Clean up buffer, remove all structs it used
void buffer_cleanup(struct buffer* buffer);

// Clean up and free buffer
void free_buffer(struct buffer* buffer);

#include "buffer-impl.h"

#endif // GRAPEFRUIT_BUFFER_H_
