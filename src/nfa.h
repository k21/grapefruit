#ifndef NEGREP_NFA_H_
#define NEGREP_NFA_H_

#include <stdbool.h>
#include <stdint.h>

#include "list.h"

struct nfa_node {
	intptr_t id;
	union {
		struct nfa_edge** edges;
		struct list edges_list;
	};
	uintptr_t edge_count;
};

struct nfa_edge {
	struct nfa_node* destination;
	uint_least8_t min, max;
};

struct nfa {
	struct list exits;
	union {
		struct nfa_node** nodes;
		struct list nodes_list;
	};
	uintptr_t node_count;
};

void free_nfa(struct nfa* nfa);

#endif // NEGREP_NFA_H_
