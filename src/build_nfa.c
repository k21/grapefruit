#include <stdlib.h>

#include "build_nfa.h"
#include "list.h"

static struct nfa* new_nfa(void) {
	struct nfa* res = malloc(sizeof(struct nfa));
	res->exits.head = res->exits.tail = 0;
	res->node_count = 0;
	return res;
}

static struct nfa_node* new_nfa_node(void) {
	struct nfa_node* res = malloc(sizeof(struct nfa_node));
	res->edges.head = res->edges.tail = 0;
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

static void add_exits(struct nfa* where, struct list* what) {
	if (!where->exits.head) {
		where->exits.head = what->head;
		where->exits.tail = what->tail;
	} else {
		where->exits.tail->next = what->head;
		where->exits.tail = what->tail;
	}
}

static void add_edges(struct nfa_node* where, struct list* what) {
	where->edges.tail->next = what->head;
	where->edges.tail = what->tail;
}

static struct nfa* new_range_nfa(char min, char max) {
	struct nfa_edge* edge = new_nfa_edge();
	edge->destination = 0;
	edge->min = min;
	edge->max = max;
	struct nfa_node* node = new_nfa_node();
	list_push(&node->edges, edge);
	struct nfa* res = new_nfa();
	res->start = node;
	list_push(&res->exits, edge);
	res->node_count = 1;
	return res;
}

struct nfa* build_nfa(struct syntree* tree) {
	struct nfa* t1;
	struct nfa* t2;
	switch (tree->type) {
		case CONCAT:
			t1 = build_nfa(tree->concat.part1);
			t2 = build_nfa(tree->concat.part2);
			set_exits(&t1->exits, t2->start);
			list_clear(&t1->exits);
			t1->exits = t2->exits;
			t1->node_count += t2->node_count;
			free(t2);
			return t1;
		case ALTER:
			t1 = build_nfa(tree->alter.option1);
			t2 = build_nfa(tree->alter.option2);
			add_exits(t1, &t2->exits);
			add_edges(t1->start, &t2->start->edges);
			t1->node_count += t2->node_count-1;
			free(t2->start);
			free(t2);
			return t1;
		case REPEAT:
			
			//TODO
			break;
		case RANGE:
			return new_range_nfa(tree->range.min, tree->range.max);
	}
	return 0;
}
