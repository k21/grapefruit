#ifndef GRAPEFRUIT_BUFFER_IMPL_H_
#define GRAPEFRUIT_BUFFER_IMPL_H_

static void buffer_chunk_cleanup(struct buffer_chunk* chunk) {
	free(chunk->data);
}

static void buffer_chunk_init(struct buffer_chunk* chunk, uintptr_t size) {
	chunk->full = 0;
	if (size != 0) {
		chunk->data = alloc(size);
	} else {
		chunk->data = 0;
	}
}

static inline int_fast8_t buffer_next(struct buffer* buffer) {
	++buffer->pos;
	if ((uintptr_t)buffer->pos < buffer->current.full) {
		return 1;
	}
	if (buffer->mark != -1) {
		struct buffer_chunk* to_save = alloc(sizeof(struct buffer_chunk));
		to_save->full = buffer->current.full;
		to_save->data = buffer->current.data;
		list_push_back(&buffer->chunks, to_save);
	} else {
		buffer_chunk_cleanup(&buffer->current);
	}
	buffer_chunk_init(&buffer->current, buffer->max_chunk_size);
	char* data = buffer->current.data;
	intptr_t res = read(buffer->fd_in, data, buffer->max_chunk_size);
	if (res == -1) {
		return -1;
	} else if (res == 0) {
		buffer->pos = -1;
		return 0;
	}
	buffer->current.full = res;
	buffer->pos = 0;
	return 1;
}

static inline uint_fast8_t buffer_get(struct buffer* buffer) {
	return buffer->current.data[buffer->pos];
}

#endif // GRAPEFRUIT_BUFFER_IMPL_H_
