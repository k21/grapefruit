#ifndef NEGREP_BUFFER_H_
#define NEGREP_BUFFER_H_

#include <stdint.h>

#include "list.h"

struct buffer {
	intptr_t full;
	char* data;
};

struct buffer* buffer_fill(int fd, uintptr_t size);
void free_buffer(struct buffer* buffer);

#endif // NEGREP_BUFFER_H_
