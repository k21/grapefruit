#include <stdlib.h>

#include "nfa.h"
#include "list.h"

void free_nfa(struct nfa* nfa) {
	list_clear(&nfa->exits);
	uintptr_t i, j;
	for (i = 0; i < nfa->node_count; ++i) {
		struct nfa_node* node = nfa->nodes.array[i];
		for (j = 0; j < node->edge_count; ++j) {
			free(node->edges.array[j]);
		}
		free(node->edges.array);
		free(node);
	}
	free(nfa->nodes.array);
	free(nfa);
}
