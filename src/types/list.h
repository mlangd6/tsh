/* list.h - Doubly linked list prototypes declaration */
#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

typedef struct list_t list_t;

/* Create an empty list */
list_t *list_create ();

/* free LIST and all its elements. 
   if FULL is true then free is used on data too */
void list_free (list_t *list, bool full);



/* Get the size of LIST.
   Return -1 if LIST is NULL */
int list_size (list_t *list);

/* Check if LIST is empty.
   Return -1 if LIST is NULL */
int list_is_empty (list_t *list);



/* Insert VAL at the beginning of LIST */
void list_insert_first (list_t *list, void *val);

/* Insert VAL at the end of LIST */
void list_insert_last (list_t *list, void *val);



/* Remove the first element of LIST.
   Returns NULL if LIST is empty otherwise the value of the first element. */
void *list_remove_first (list_t *list);

/* Remove the last element of LIST.
   Returns NULL if LIST is empty otherwise the value of the last element. */
void *list_remove_last (list_t *list);



/* Get the first element of LIST
   Returns NULL if LIST is empty otherwise the value of the first element. */
void *list_first (list_t *list);

/* Get the last element of LIST
   Returns NULL if LIST is empty otherwise the value of the last element. */
void *list_last (list_t *list);



/* Apply F to all elements of LIST */
void list_iter (list_t *list, void (*f)(void *));

  
#endif
