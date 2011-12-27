#ifndef GRAPEFRUIT_LIST_H_
#define GRAPEFRUIT_LIST_H_

struct list_node {
	void* ptr;
	struct list_node* next;
};

// linked list of pointers to any kind of data
struct list {
	struct list_node* head;
	struct list_node* tail;
};

// Allocate and initialize new list
struct list* new_list(void);

// Push new pointer to the front/back of the list
void list_push_front(struct list* list, void* what);
void list_push_back(struct list* list, void* what);

// Pop and return pointer from the front of the list
void* list_pop_front(struct list* list);

// Clear whole list and call free() on all pointers in it
void list_purge(struct list* list);

// Remove all pointers from list
void list_clear(struct list* list);

// Remove all pointers frfom l2 and add them to l1
void list_join(struct list* l1, struct list* l2);

#endif // GRAPEFRUIT_LIST_H_
