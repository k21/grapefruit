#ifndef NEGREP_NFA_H_
#define NEGREP_NFA_H_

#include <stdbool.h>

#include "list.h"

struct nfa_node {
	union {
		struct nfa_edge** edges;
		struct list edges_list;
	};
	int edge_count;
};

struct nfa_edge {
	struct nfa_node* destination;
	char min, max;
};

struct nfa {
	struct list exits;
	union {
		struct nfa_node** nodes;
		struct list nodes_list;
	};
	int node_count;
};

void free_nfa(struct nfa* nfa);

#endif // NEGREP_NFA_H_
