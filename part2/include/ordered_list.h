#ifndef __ORDERED_LIST_H__
#define __ORDERED_LIST_H__

#include <list.h>

typedef struct {
    struct list_head head; 
    int (*compare)(void *a, void *b); // Comparison function for ordering
} OrderedList;

void init_list(OrderedList* list, int (*compare)(void *a, void *b));

void insert_ordered(OrderedList* list, void* element);

void* pop(OrderedList* list);

void* get_first_ordered(OrderedList* list);

void delete_from_ordered_list(OrderedList* list, void* element);

int ordered_list_empty(OrderedList* list);

#endif /* __ORDERED_LIST_H__ */



