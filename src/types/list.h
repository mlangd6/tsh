/* list.h - Doubly linked list prototypes declaration */
#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

typedef struct list list;

/* Create an empty list */
list *list_create ();

/* free LIST and all its elements.
   if FULL is true then free is used on data too */
void list_free (list *list, bool full);



/* Get the size of LIST.
   Return -1 if LIST is NULL */
int list_size (list *list);

/* Check if LIST is empty.
   Return -1 if LIST is NULL */
int list_is_empty (list *list);



/* Insert VAL at the beginning of LIST */
void list_insert_first (list *list, void *val);

/* Insert VAL at the end of LIST */
void list_insert_last (list *list, void *val);



/* Remove the first element of LIST.
   Returns NULL if LIST is empty otherwise the value of the first element. */
void *list_remove_first (list *list);

/* Remove the last element of LIST.
   Returns NULL if LIST is empty otherwise the value of the last element. */
void *list_remove_last (list *list);



/* Get the first element of LIST
   Returns NULL if LIST is empty otherwise the value of the first element. */
void *list_first (list *list);

/* Get the last element of LIST
   Returns NULL if LIST is empty otherwise the value of the last element. */
void *list_last (list *list);



/* Apply F to all elements of LIST */
void list_iter (list *list, void (*f)(void *));

/**
 * Check if all the elements of the list satisfies a predicate
 * @param list the list
 * @param predicate the predicate to test on all the elements of the list
 * @return true if predicate returns true on all of the elements, else false
 */
bool list_for_all(list *list, bool (*predicate)(void *));


#endif
