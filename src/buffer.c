#include <errno.h>
#include <unistd.h>

#include "buffer.h"
#include "common.h"
#include "list.h"

void buffer_init(struct buffer* buffer, int in, int out, uintptr_t size) {
	buffer->fd_in = in;
	buffer->fd_out = out;
	buffer->max_chunk_size = size;
	buffer->pos = -1;
	buffer->mark = -1;
	buffer->chunks.head = buffer->chunks.tail = 0;
	buffer_chunk_init(&buffer->current, 0);
}

struct buffer* new_buffer(int in, int out, uintptr_t size) {
	struct buffer* res = alloc(sizeof(struct buffer));
	buffer_init(res, in, out, size);
	return res;
}

static void free_buffer_chunk(struct buffer_chunk* chunk) {
	buffer_chunk_cleanup(chunk);
	free(chunk);
}

void buffer_mark(struct buffer* buffer) {
	buffer->mark = buffer->pos;
	struct list_node* node = buffer->chunks.head;
	while (node) {
		free_buffer_chunk(node->ptr);
		node = node->next;
	}
	list_clear(&buffer->chunks);
}

// Repeatedly try to write until write succeeds of an error occurs
static intptr_t write_all(int fd, char* buffer,
		uintptr_t begin, uintptr_t end) {
	uintptr_t done = begin;
	while (done != end) {
		intptr_t res = write(fd, buffer+done, end-done);
		if (res == -1 && errno == EINTR) continue;
		if (res == -1) return -1;
		if (res == 0) return done;
		done += res;
	}
	return done;
}

int_fast8_t buffer_print(struct buffer* buffer, bool print_current) {
	struct list_node* node = buffer->chunks.head;
	// print contents of all chunks
	while (node) {
		struct buffer_chunk* chunk = node->ptr;
		intptr_t res = write_all(buffer->fd_out,
				chunk->data, buffer->mark, chunk->full);
		if (res == -1) return -1;
		else if ((uintptr_t)res != chunk->full) return 0;
		buffer->mark = 0;
		free_buffer_chunk(node->ptr);
		node = node->next;
	}
	list_clear(&buffer->chunks);

	// if there was no data read, return success
	if (buffer->pos == -1) return 1;
	// if we are done, return success
	if (buffer->pos == 0 && !print_current) return 1;

	uintptr_t end = buffer->pos+(print_current?1:0);
	intptr_t res = write_all(buffer->fd_out,
			buffer->current.data, buffer->mark, end);
	if (res == -1) return -1;
	else if ((uintptr_t)res != end) return 0;

	// remove mark
	buffer->mark = -1;
	return 1;
}

void buffer_cleanup(struct buffer* buffer) {
	struct list_node* node = buffer->chunks.head;
	while (node) {
		free_buffer_chunk(node->ptr);
		node = node->next;
	}
	list_clear(&buffer->chunks);
	buffer_chunk_cleanup(&buffer->current);

}

void free_buffer(struct buffer* buffer) {
	buffer_cleanup(buffer);
	free(buffer);
}
