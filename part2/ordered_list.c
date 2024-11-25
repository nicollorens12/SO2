#include "ordered_list.h"
#include <types.h>

void init_list(OrderedList* list, int (*compare)(void *a, void *b)) {
    INIT_LIST_HEAD(&list->head);
    list->compare = compare;
}

void insert_ordered(OrderedList* list, void* element) {
    struct list_head *pos;
    list_for_each(pos, &list->head) {
        if (list->compare(element, pos) < 0) {
            list_add_tail(element, pos);
            return;
        }
    }
    list_add_tail(element, &list->head);
}

void pop(OrderedList* list) {
    if (ordered_list_empty(list)) {
        return NULL;
    }
    struct list_head* first = list->head.next;
    list_del(first);
}

struct list_head* get_first_ordered(OrderedList* list) {
    if (ordered_list_empty(list)) {
        return NULL;
    }
    return list->head.next;
}

void delete_from_ordered_list(OrderedList* list, void* element) {
    list_del(element);
}

int ordered_list_empty(OrderedList* list) {
    return list_empty(&list->head);
}