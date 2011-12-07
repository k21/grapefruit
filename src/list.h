#ifndef GRAPEFRUIT_LIST_H_
#define GRAPEFRUIT_LIST_H_

struct list_node {
	void* ptr;
	struct list_node* next;
};

struct list {
	struct list_node* head;
	struct list_node* tail;
};

struct list* new_list(void);
void list_push_front(struct list* list, void* what);
void list_push_back(struct list* list, void* what);
void* list_pop_front(struct list* list);
void list_purge(struct list* list);
void list_clear(struct list* list);
void list_join(struct list* l1, struct list* l2);

#endif // GRAPEFRUIT_LIST_H_
