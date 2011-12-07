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

struct buffer {
	int fd_in, fd_out;
	uintptr_t max_chunk_size;
	intptr_t pos;
	intptr_t mark;
	struct list chunks;
	struct buffer_chunk* current;
};

struct buffer* new_buffer(int in, int out, uintptr_t size);
static struct buffer_chunk* new_chunk(uintptr_t size);
static inline int_fast8_t buffer_next(struct buffer* buffer);
static inline inline uint_fast8_t buffer_get(struct buffer* buffer);
void buffer_mark(struct buffer* buffer);
int_fast8_t buffer_print(struct buffer* buffer, bool print_current);
static void free_buffer_chunk(struct buffer_chunk* chunk);
void free_buffer(struct buffer* buffer);

#include "buffer-impl.h"

#endif // GRAPEFRUIT_BUFFER_H_
