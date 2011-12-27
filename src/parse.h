#ifndef GRAPEFRUIT_PARSE_H_
#define GRAPEFRUIT_PARSE_H_

#include "syntree.h"

// Parse regular expression, return it as abstract syntax tree
struct syntree* parse(char* re, uintptr_t len);

#endif // GRAPEFRUIT_PARSE_H_
