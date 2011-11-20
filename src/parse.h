#ifndef NEGREP_PARSE_H_
#define NEGREP_PARSE_H_

#include "syntree.h"

struct syntree* parse(char* re, uintptr_t len);

#endif // NEGREP_PARSE_H_
