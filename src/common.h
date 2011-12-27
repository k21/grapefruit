#ifndef GRAPEFRUIT_COMMON_H_
#define GRAPEFRUIT_COMMON_H_

#include <error.h>
#include <stdint.h>
#include <stdlib.h>

void* alloc_(uintptr_t size, const char* filename, unsigned int linenum);

// Attempt to allocate memory
// If the attempt is unsuccessfull, print error meassage and exit
#define alloc(size) \
		alloc_((size), __FILE__, __LINE__)

// Print error message and exit
#define die(status, errnum, ...) \
		error_at_line((status), (errnum), __FILE__, __LINE__, __VA_ARGS__)

#endif // GRAPEFRUIT_COMMON_H_
