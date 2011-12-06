#include <errno.h>
#include <unistd.h>

#include "buffer.h"
#include "common.h"

struct buffer* buffer_fill(int fd, uintptr_t size) {
	struct buffer* res = alloc(sizeof(struct buffer));
	res->data = alloc(sizeof(char)*size);
	res->full = 0;
	while (1) {
		res->full = read(fd, res->data, size);
		if (res->full == -1 && errno == EINTR) continue;
		else break;
	}
	if (res->full == 0) {
		free(res->data);
		res->data = 0;
	} else {
		res->data = realloc(res->data, res->full);
	}
	return res;
}

void free_buffer(struct buffer* buffer) {
	free(buffer->data);
	free(buffer);
}
