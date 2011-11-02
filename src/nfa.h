#ifndef NEGREP_NFA_H_
#define NEGREP_NFA_H_

#include <stdbool.h>

#include "list.h"

struct nfa_node {
	struct list edges;
};

struct nfa_edge {
	struct nfa_node* destination;
	char min, max;
};

struct nfa {
	struct nfa_node* start;
	struct list exits;
};

void free_nfa(struct nfa* nfa);

#endif // NEGREP_NFA_H_
