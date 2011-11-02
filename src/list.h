#ifndef NEGREP_LIST_H_
#define NEGREP_LIST_H_

struct list_node {
	void* ptr;
	struct list_node* next;
};

struct list {
	struct list_node* head;
	struct list_node* tail;
};

struct list* new_list();
void list_push(struct list* list, void* what);
void list_purge(struct list* list);
void list_free(struct list* list);

#endif // NEGREP_LIST_H_
