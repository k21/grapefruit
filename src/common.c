#include <errno.h>
#include <error.h>
#include <inttypes.h>
#include <stdint.h>

#include "common.h"

void* alloc_(uintptr_t size, const char* filename, unsigned int linenum) {
	void* res = malloc(size);
	if (!res) {
		error_at_line(2, errno, filename, linenum,
				"Could not allocate %"PRIuPTR" bytes of memory", size);
	}
	return res;
}
