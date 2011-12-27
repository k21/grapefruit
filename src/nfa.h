#ifndef GRAPEFRUIT_NFA_H_
#define GRAPEFRUIT_NFA_H_

#include <stdbool.h>
#include <stdint.h>

#include "list.h"

// one node in NFA, its id is unique in the whole automaton
struct nfa_node {
	intptr_t id;
	union {
		struct nfa_edge** array;
		struct list list;
	} edges;
	uintptr_t edge_count;
};

// one edge in NFA
// min and max is inclusive range of accepted characters
struct nfa_edge {
	struct nfa_node* destination;
	uint_least8_t min, max;
};

// nfa represents regular expression as non-deterministic finite automaton
// nodes.list is only used during building
// enter node is always the first node
// exit node is virtual, in edges it is represented by null pointers
// exit node is NOT included in the node_count
// exits holds all edges pointing to exit node
struct nfa {
	struct list exits;
	union {
		struct nfa_node** array;
		struct list list;
	} nodes;
	uintptr_t node_count;
};

// Free all structures used by the nfa
void free_nfa(struct nfa* nfa);

#endif // GRAPEFRUIT_NFA_H_
