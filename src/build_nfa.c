#include <stdlib.h>

#include "build_nfa.h"
#include "common.h"
#include "list.h"

static struct nfa* new_nfa(void) {
	struct nfa* res = alloc(sizeof(struct nfa));
	res->exits.head = res->exits.tail = 0;
	res->nodes_list.head = res->nodes_list.tail = 0;
	res->node_count = 0;
	return res;
}

static struct nfa_node* new_nfa_node(void) {
	struct nfa_node* res = alloc(sizeof(struct nfa_node));
	res->id = -1;
	res->edges_list.head = res->edges_list.tail = 0;
	res->edge_count = 0;
	return res;
}

static struct nfa_edge* new_nfa_edge(void) {
	return alloc(sizeof(struct nfa_edge));
}

static void set_exits(struct list* exits, struct nfa_node* to) {
	struct list_node* node = exits->head;
	while (node) {
		struct nfa_edge* edge = node->ptr;
		edge->destination = to;
		node = node->next;
	}
}

static struct nfa* new_range_nfa(char min, char max) {
	struct nfa_edge* edge = new_nfa_edge();
	edge->destination = 0;
	edge->min = min;
	edge->max = max;
	struct nfa_node* node = new_nfa_node();
	list_push_back(&node->edges_list, edge);
	node->edge_count = 1;
	struct nfa* res = new_nfa();
	list_push_back(&res->exits, edge);
	list_push_back(&res->nodes_list, node);
	res->node_count = 1;
	return res;
}

static struct nfa* new_empty_nfa(void) {
	return new_range_nfa(1, 0);
}

static struct nfa_node* start(struct nfa* nfa) {
	return nfa->nodes_list.head->ptr;
}

static struct nfa* build_nfa_impl(struct syntree* tree);

static struct nfa* concat_nfas(struct nfa* n1, struct nfa* n2) {
	set_exits(&n1->exits, start(n2));
	list_clear(&n1->exits);
	n1->exits = n2->exits;
	list_join(&n1->nodes_list, &n2->nodes_list);
	n1->node_count += n2->node_count;
	free(n2);
	return n1;
}

static struct nfa* concat_trees(struct syntree* t1, struct syntree* t2) {
	struct nfa* n1 = build_nfa_impl(t1);
	struct nfa* n2 = build_nfa_impl(t2);
	return concat_nfas(n1, n2);
}

static struct nfa* alter_nfas(struct nfa* n1, struct nfa* n2) {
	struct nfa_node* start1 = start(n1);
	struct nfa_node* start2 = start(n2);
	list_join(&n1->exits, &n2->exits);
	list_join(&start1->edges_list, &start2->edges_list);
	start1->edge_count += start2->edge_count;
	list_pop_front(&n2->nodes_list);
	list_join(&n1->nodes_list, &n2->nodes_list);
	n1->node_count += n2->node_count-1;
	free(start2);
	free(n2);
	return n1;
}

static struct nfa* alter_trees(struct syntree* t1, struct syntree* t2) {
	struct nfa* n1 = build_nfa_impl(t1);
	struct nfa* n2 = build_nfa_impl(t2);
	return alter_nfas(n1, n2);
}

static struct nfa* repeat_tree(struct syntree* repeated, int min, int max) {
	struct nfa* res = 0;
	while (min > 1) {
		if (!res) {
			res = build_nfa_impl(repeated);
		} else {
			struct nfa* n = build_nfa_impl(repeated);
			res = concat_nfas(res, n);
		}
		--min;
		if (max != -1) --max;
	}
	if (max == -1) {
		struct nfa* optional = build_nfa_impl(repeated);
		struct nfa_edge* back_edge = new_nfa_edge();
		back_edge->destination = start(optional);
		back_edge->min = 1; back_edge->max = 0;
		struct nfa_edge* exit_edge = new_nfa_edge();
		exit_edge->destination = 0;
		exit_edge->min = 1; exit_edge->max = 0;
		struct nfa_node* exit_node = new_nfa_node();
		list_push_back(&exit_node->edges_list, back_edge);
		list_push_back(&exit_node->edges_list, exit_edge);
		exit_node->edge_count = 2;
		set_exits(&optional->exits, exit_node);
		list_clear(&optional->exits);
		list_push_back(&optional->nodes_list, exit_node);
		optional->node_count += 1;
		list_push_back(&optional->exits, exit_edge);
		if (!res) {
			res = optional;
		} else {
			res = concat_nfas(res, optional);
		}
	} else {
		struct nfa* optional = build_nfa_impl(repeated);
		while (max > 1) {
			optional = alter_nfas(optional, new_empty_nfa());
			optional = concat_nfas(optional, build_nfa_impl(repeated));
			--max;
		}
		if (!res) {
			res = optional;
		} else {
			res = concat_nfas(res, optional);
		}
	}
	if (min == 0 && res) {
		res = alter_nfas(res, new_empty_nfa());
	}
	return res ? res : new_empty_nfa();
}

static struct nfa* build_nfa_impl(struct syntree* tree) {
	switch (tree->type) {
		case CONCAT:
			return concat_trees(tree->concat.part1, tree->concat.part2);
		case ALTER:
			return alter_trees(tree->alter.option1, tree->alter.option2);
		case REPEAT:
			return repeat_tree(tree->repeat.repeated,
					tree->repeat.min, tree->repeat.max);
		case RANGE:
			return new_range_nfa(tree->range.min, tree->range.max);
		case EMPTY:
			return new_empty_nfa();
	}
	return 0;
}

static void copy_list_to_array(struct list* list, void** array) {
	struct list_node* node = list->head;
	int i = 0;
	while (node) {
		array[i++] = node->ptr;
		node = node->next;
	}
}

struct nfa* build_nfa(struct syntree* tree) {
	struct nfa* nfa = build_nfa_impl(tree);
	struct nfa_node** nodes = alloc(nfa->node_count * sizeof(void*));
	copy_list_to_array(&nfa->nodes_list, (void**)nodes);
	list_clear(&nfa->nodes_list);
	nfa->nodes = nodes;
	int i;
	for (i = 0; i < nfa->node_count; ++i) {
		struct nfa_node* node = nfa->nodes[i];
		node->id = i;
		struct nfa_edge** edges = alloc(node->edge_count * sizeof(void*));
		copy_list_to_array(&node->edges_list, (void**)edges);
		list_clear(&node->edges_list);
		node->edges = edges;
	}
	return nfa;
}
