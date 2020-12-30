/**
 * @file list.h 
 * Doubly linked list data structure
 */

#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

/**
 * Doubly linked list.
 * Note that the list holds only address and not the pointed data
 */ 
typedef struct list list;

/**
 * Create an empty list 
 * @return a malloc'd empty list
 */
list *list_create ();

/**
 * Free the memory allocated for a list
 * @param list a list
 * @param full if `full` is `true` then free is used on data too 
 */
void list_free (list *list, bool full);

/**
 * Get the size of a list
 * @param list a list
 * @return the size of the list or -1 if `list` is `NULL`
 */
int list_size (list *list);

/**
 * Check if a list is empty
 * @param list a list
 * @return 
 * * 0 if `list` is no empty
 * * 1 if `list` is empty
 * * -1 if `list` is `NULL`
 */
int list_is_empty (list *list);


/**
 * Insert value a the beginning of a list
 * @param list a list
 * @param val the address to insert
 */
void list_insert_first (list *list, void *val);

/**
 * Insert value a the end of a list
 * @param list a list
 * @param val the address to insert
 */
void list_insert_last (list *list, void *val);



/**
 * Remove the first element of a list
 * @param list a list
 * @return `NULL` if `list` is empty; otherwise the address of the first element.
 */
void *list_remove_first (list *list);

/**
 * Remove the last element of a list
 * @param list a list
 * @return `NULL` if `list` is empty; otherwise the address of the last element.
 */
void *list_remove_last (list *list);


/**
 * Get the first element of a list
 * @param list a list
 * @return `NULL` if `list` is empty; otherwise the address of the first element.
 */
void *list_first (list *list);

/**
 * Get the last element of a list
 * @param list a list
 * @return `NULL` if `list` is empty; otherwise the address of the last element.
 */
void *list_last (list *list);


/**
 * Apply a function to all elements of a list
 * @param list a list
 * @param f a function to apply to each element in `list`
 */
void list_iter (list *list, void (*f)(void *));

/**
 * Check if all the elements of the list satisfies a predicate
 * @param list the list
 * @param predicate the predicate to test on all the elements of the list
 * @return true if predicate returns true on all of the elements, else false
 */
bool list_for_all(list *list, bool (*predicate)(void *));

/**
 * Free all the element in a list
 * @param list the list to be free'd
 * @param free_func the function to launch that free the elements of the list
 */
void list_free_full(list *list, void (*free_func)(void *));

#endif
