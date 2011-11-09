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

void list_push(struct list* list, void* what) {
	struct list_node* node = new_node();
	node->ptr = what;
	node->next = list->head;
	if (!list->head) {
		list->head = list->tail = node;
	} else {
		list->head = node;
	}
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
