#ifndef NEGREP_COMMON_H_
#define NEGREP_COMMON_H_

#include <error.h>
#include <stdlib.h>

void* alloc_(size_t size, const char* filename, unsigned int linenum);

#define alloc(size) \
		alloc_((size), __FILE__, __LINE__)

#endif // NEGREP_COMMON_H_
