/**
 * @file array.h
 * Array data structure
 */

#ifndef ARRAY_H
#define ARRAY_H

#include <stdbool.h>
#include <stddef.h>

typedef struct array array;

/** 
 * Create an empty array
 *
 * @param elem_size the size of each element in bytes
 * @return a malloc'd empty array
 */
array *array_create (size_t elem_size);

/**
 * Free the memory allocated for an array.
 *
 * @param arr an array
 * @param full If `full` is `true` then memory block holding the elements are freed too. 
 */
void array_free (array *arr, bool full);

/**
 * Gets the size of an array
 *
 * @param arr an array
 * @return the size of the array or -1 if `arr` is `NULL`
 */
int array_size (array *arr);

/**
 * Sets the element at the given index in an array with `val`.
 *
 * @param arr an array
 * @param i the index of the element to change
 * @param val a pointer to the new element
 * @return a malloc'd pointer to the old content
 */
void *array_set (array *arr, size_t i, void *val);

/**
 * Inserts an element at the given index in an array.
 *
 * If `i` is equal to the array’s current size, the array is expanded by 1.
 *
 * If `i` is less than the array’s current size, the new element will be inserted into the array, and the existing entries above `i` will be moved upwards.
 *
 * @param arr an array
 * @param i the index to place the element at
 * @param val a pointer to the element to insert
 */
void array_insert (array *arr, size_t i, void *val);

/**
 * Inserts an element at the beginning of an array.
 *
 * @param arr an array
 * @param val a pointer to the element to insert
 */
void array_insert_first (array *arr, void *val);

/**
 * Inserts an element at the end of an array.
 *
 * @param arr an array
 * @param val a pointer to the element to insert
 */
void array_insert_last (array *arr, void *val);


/**
 * Gets the element at the given index in an array.
 *
 * @param arr an array
 * @param i the index of the element to access
 * @return a malloc'd pointer to a copy of the element
 */
void *array_get (array *arr, size_t i);

/**
 * Removes the element at the given index in an array. The following elements are moved down one place.
 *
 * @param arr an array
 * @param i the index of the element to remove
 * @return a malloc'd pointer to the removed element
 */
void *array_remove (array *arr, size_t i);

/**
 * Removes the first element of an array. The following elements are moved down one place.
 *
 * @param arr an array
 * @return a malloc'd pointer to the removed element
 */
void *array_remove_first (array *arr);

/**
 * Removes the last element of an array.
 *
 * @param arr an array
 * @return a malloc'd pointer to the removed element
 */
void *array_remove_last (array *arr);

/** Sorts an array using `comp` which should be a qsort()-style comparison function (returns less than zero for first arg is less than second arg, zero for equal, greater zero if first arg is greater than second arg).
 * @param arr an array
 * @param comp a comparison function
 */
void array_sort (array *arr, int (*comp)(const void *, const void *));

#endif
