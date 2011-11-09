#include <stdlib.h>

#include "nfa.h"
#include "list.h"

static void collect(struct nfa_node* node, struct list* list) {
	if (!node) return;
	if (!node->edges.head) return;
	list_push(list, node);
	struct list_node* n = node->edges.head;
	node->edges.head = 0;
	while (n) {
		list_push(list, n);
		struct nfa_edge* edge = n->ptr;
		list_push(list, edge);
		collect(edge->destination, list);
		n = n->next;
	}
}

void free_nfa(struct nfa* nfa) {
	list_clear(&nfa->exits);
	struct list* list = new_list();
	list_push(list, nfa);
	collect(nfa->start, list);
	list_purge(list);
	free(list);
}
