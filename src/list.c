#include <stdlib.h>

#include "list.h"

static struct list_node* new_node(void) {
	return malloc(sizeof(struct list_node));
}

struct list* new_list() {
	struct list* res = malloc(sizeof(struct list));
	res->head = res->tail = 0;
	return res;
}

void list_push_back(struct list* list, void* what) {
	struct list_node* node = new_node();
	node->ptr = what;
	node->next = list->head;
	if (!list->head) {
		list->head = list->tail = node;
	} else {
		list->head = node;
	}
}

void* list_pop_front(struct list* list) {
	if (!list->head) return 0;
	struct list_node* front = list->head;
	list->head = front->next;
	if (!list->head) list->tail = 0;
	void* res = front->ptr;
	free(front);
	return res;
}

void list_purge(struct list* list) {
	struct list_node* node = list->head;
	struct list_node* next;
	while (node) {
		next = node->next;
		free(node->ptr);
		free(node);
		node = next;
	}
}

void list_clear(struct list* list) {
	struct list_node* node = list->head;
	struct list_node* next;
	while (node) {
		next = node->next;
		free(node);
		node = next;
	}
	list->head = list->tail = 0;
}

void list_join(struct list* l1, struct list* l2) {
	if (!l1->head) {
		l1->head = l2->head;
		l1->tail = l2->tail;
	} else if (l2->head) {
		l1->tail->next = l2->head;
		l1->tail = l2->tail;
	}
	l2->head = l2->tail = 0;
}
