#include <errno.h>
#include <error.h>

#include "common.h"

void* alloc_(size_t size, const char* filename, unsigned int linenum) {
	void* res = malloc(size);
	if (!res) {
		error_at_line(1, errno, filename, linenum,
				"Could not allocate %d bytes of memory", size);
	}
	return res;
}
