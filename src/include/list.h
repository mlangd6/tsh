/* list.h - Double linked list prototypes declaration */
#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

typedef struct list_t list_t;

/* Create an empty list */
list_t *create_list();

/* free all elements used by LIST. */
void free_list(list_t *list, bool full);

void list_add_first(list_t *list, void *val);

#endif
