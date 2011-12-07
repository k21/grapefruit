#ifndef NEGREP_BUFFER_IMPL_H_
#define NEGREP_BUFFER_IMPL_H_

static void free_buffer_chunk(struct buffer_chunk* chunk) {
	free(chunk->data);
	free(chunk);
}

static struct buffer_chunk* new_chunk(uintptr_t size) {
	struct buffer_chunk* res = alloc(sizeof(struct buffer_chunk));
	res->full = 0;
	res->data = alloc(size);
	return res;
}

static inline int_fast8_t buffer_next(struct buffer* buffer) {
	++buffer->pos;
	if (buffer->current && (uintptr_t)buffer->pos < buffer->current->full) {
		return 1;
	}
	if (buffer->mark != -1) {
		list_push_back(&buffer->chunks, buffer->current);
	} else if (buffer->current) {
		free_buffer_chunk(buffer->current);
	}
	buffer->current = new_chunk(buffer->max_chunk_size);
	char* data = buffer->current->data;
	intptr_t res = read(buffer->fd_in, data, buffer->max_chunk_size);
	if (res == -1) {
		return -1;
	} else if (res == 0) {
		buffer->pos = -1;
		return 0;
	}
	buffer->current->full = res;
	buffer->pos = 0;
	return 1;
}

static inline uint_fast8_t buffer_get(struct buffer* buffer) {
	return buffer->current->data[buffer->pos];
}

#endif // NEGREP_BUFFER_IMPL_H_
