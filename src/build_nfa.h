#ifndef GRAPEFRUIT_BUILD_NFA_H_
#define GRAPEFRUIT_BUILD_NFA_H_

#include "syntree.h"
#include "nfa.h"

// Build nfa from abstract syntax tree representation of regular expression
struct nfa* build_nfa(struct syntree* tree, bool whole_lines);

#endif // GRAPEFRUIT_BUILD_NFA_H_
