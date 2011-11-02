#ifndef NEGREP_BUILD_NFA_H_
#define NEGREP_BUILD_NFA_H_

#include "syntree.h"
#include "nfa.h"

struct nfa* build_nfa(struct syntree* tree);

#endif // NEGREP_BUILD_NFA_H_
