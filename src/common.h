#ifndef GRAPEFRUIT_COMMON_H_
#define GRAPEFRUIT_COMMON_H_

#include <error.h>
#include <stdint.h>
#include <stdlib.h>

void* alloc_(uintptr_t size, const char* filename, unsigned int linenum);

#define alloc(size) \
		alloc_((size), __FILE__, __LINE__)

#define die(status, errnum, ...) \
		error_at_line((status), (errnum), __FILE__, __LINE__, __VA_ARGS__)

#endif // GRAPEFRUIT_COMMON_H_
