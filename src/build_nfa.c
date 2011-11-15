#include <stdlib.h>

#include "build_nfa.h"
#include "list.h"

static struct nfa* new_nfa(void) {
	struct nfa* res = malloc(sizeof(struct nfa));
	res->exits.head = res->exits.tail = 0;
	res->nodes_list.head = res->nodes_list.tail = 0;
	res->node_count = 0;
	return res;
}

static struct nfa_node* new_nfa_node(void) {
	struct nfa_node* res = malloc(sizeof(struct nfa_node));
	res->id = -1;
	res->edges_list.head = res->edges_list.tail = 0;
	res->edge_count = 0;
	return res;
}

static struct nfa_edge* new_nfa_edge(void) {
	return malloc(sizeof(struct nfa_edge));
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

static struct nfa_node* start(struct nfa* nfa) {
	return nfa->nodes_list.head->ptr;
}

static struct nfa* build_nfa_impl(struct syntree* tree) {
	switch (tree->type) {
		case CONCAT: {
			struct nfa* t1 = build_nfa_impl(tree->concat.part1);
			struct nfa* t2 = build_nfa_impl(tree->concat.part2);
			set_exits(&t1->exits, start(t2));
			list_clear(&t1->exits);
			t1->exits = t2->exits;
			list_join(&t1->nodes_list, &t2->nodes_list);
			t1->node_count += t2->node_count;
			free(t2);
			return t1;
		}
		case ALTER: {
			struct nfa* t1 = build_nfa_impl(tree->alter.option1);
			struct nfa* t2 = build_nfa_impl(tree->alter.option2);
			struct nfa_node* start1 = start(t1);
			struct nfa_node* start2 = start(t2);
			list_join(&t1->exits, &t2->exits);
			list_join(&start1->edges_list, &start2->edges_list);
			start1->edge_count += start2->edge_count;
			list_pop_front(&t2->nodes_list);
			list_join(&t1->nodes_list, &t2->nodes_list);
			t1->node_count += t2->node_count-1;
			free(start2);
			free(t2);
			return t1;
		}
		case REPEAT:
			
			//TODO
			break;
		case RANGE:
			return new_range_nfa(tree->range.min, tree->range.max);
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
	struct nfa_node** nodes = malloc(nfa->node_count * sizeof(void*));
	copy_list_to_array(&nfa->nodes_list, (void**)nodes);
	list_clear(&nfa->nodes_list);
	nfa->nodes = nodes;
	int i;
	for (i = 0; i < nfa->node_count; ++i) {
		struct nfa_node* node = nfa->nodes[i];
		node->id = i;
		struct nfa_edge** edges = malloc(node->edge_count * sizeof(void*));
		copy_list_to_array(&node->edges_list, (void**)edges);
		list_clear(&node->edges_list);
		node->edges = edges;
	}
	return nfa;
}
