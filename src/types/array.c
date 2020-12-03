#include "array.h"

#define ARRAY_INITIAL_CAPACITY 16

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct array
{
  size_t size;
  
  size_t capacity;

  size_t elem_size;

  char *data;
};



static void array_resize (array *arr, size_t new_capacity)
{
  arr->data = realloc(arr->data, arr->elem_size * new_capacity);
  assert(arr->data);

  arr->capacity = new_capacity;
}



/** 
 * Creates an empty array.
 *
 * @param elem_size the size of each element in bytes
 * @return the new empty array
 */
array *array_create (size_t elem_size)
{
  if (!elem_size)
    return NULL;
  
  array *arr = malloc(sizeof(array));
  assert (arr);

  arr->size = 0;
  arr->capacity = ARRAY_INITIAL_CAPACITY;
  arr->elem_size = elem_size;
  arr->data = malloc(arr->capacity * elem_size);
  assert (arr->data);
  
  return arr;
}


/**
 * Frees the memory allocated for an array.
 *
 * @param arr an array
 * @param full If `full` is `true` it frees the memory block holding the elements as well. 
 */
void array_free (array *arr, bool full)
{
  if (!arr)
    return;

  if (full)
    {
      for (int i=0; i < arr->size; i++)
	free(arr->data+i);
    }

  free(arr->data);
  free(arr);
}


/**
 * Gets the size of an array.
 *
 * @param arr an array
 * @return the size of the array or -1 if `arr` is `NULL`
 */
int array_size (array *arr)
{
  return arr ? arr->size : -1;
}


/**
 * Sets the element at the given index in an array with `val`.
 *
 * @param arr an array
 * @param i the index of the element to change
 * @param val a pointer to the new element
 * @return a malloc'd pointer to the old content
 */
void *array_set (array *arr, size_t i, void *val)
{
  if (!arr || arr->size <= i)
    return NULL;

  void *ret = malloc(arr->elem_size);
  assert(ret);
  
  memmove(ret, arr->data + i*arr->elem_size, arr->elem_size); // on copie
  
  memmove (arr->data + i*arr->elem_size, val, arr->elem_size); // on remplace

  return ret;
}

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
void array_insert (array *arr, size_t i, void *val)
{
  if (!arr || arr->size < i)
    return;

  if (arr->size == arr->capacity)
    array_resize (arr, 2 * arr->size);

  memmove (arr->data + (i+1)*arr->elem_size, arr->data + i*arr->elem_size, (arr->size - i)*arr->elem_size);
  memmove (arr->data + i*arr->elem_size, val, arr->elem_size);
  arr->size++;
}

/**
 * Inserts an element at the beginning of an array.
 *
 * @param arr an array
 * @val a pointer to the element to insert
 */
void array_insert_first (array *arr, void *val)
{
  array_insert (arr, 0, val);
}

/**
 * Inserts an element at the end of an array.
 *
 * @param arr an array
 * @param val a pointer to the element to insert
 */
void array_insert_last (array *arr, void *val)
{
  array_insert (arr, arr->size, val);
}


/**
 * Gets the element at the given index in an array.
 *
 * @param arr an array
 * @param i the index of the element to access
 * @return a malloc'd pointer to a copy of the element
 */
void *array_get (array *arr, size_t i)
{
  if (!arr || arr->size <= i)
    return NULL;
  
  void *ret = malloc(arr->elem_size);
  assert(ret);
  
  memmove(ret, arr->data + i*arr->elem_size, arr->elem_size);

  return ret;
}

/**
 * Removes the element at the given index in an array. The following elements are moved down one place.
 *
 * @param arr an array
 * @param i the index of the element to remove
 * @return a malloc'd pointer to the removed element
 */
void *array_remove (array *arr, size_t i)
{
  if (!arr || arr->size <= i)
    return NULL;

  void *ret = malloc(arr->elem_size);
  assert(ret);
  memmove(ret, arr->data + i*arr->elem_size, arr->elem_size);
  
  memmove (arr->data + i*arr->elem_size, arr->data + (1+i)*arr->elem_size, (arr->size - (i+1))*arr->elem_size);

  arr->size--;
  
  if (arr->capacity >= 4 * arr->size && arr->capacity > ARRAY_INITIAL_CAPACITY)
    array_resize(arr, arr->capacity / 2);
  
  return ret;
}

/**
 * Removes the first element of an array. The following elements are moved down one place.
 *
 * @param arr an array
 * @return a malloc'd pointer to the removed element
 */
void *array_remove_first (array *arr)
{
  return array_remove (arr, 0);
}

/**
 * Removes the last element of an array.
 *
 * @param arr an array
 * @return a malloc'd pointer to the removed element
 */
void *array_remove_last (array *arr)
{
  return array_remove (arr, arr->size-1);
}


/** Sorts an array using `comp` which should be a qsort()-style comparison function (returns less than zero for first arg is less than second arg, zero for equal, greater zero if first arg is greater than second arg).
 * @param arr an array
 * @param comp a comparison function
 */
void array_sort (array *arr, int (*comp)(const void *, const void *))
{
  qsort (arr->data, arr->size, arr->elem_size, comp);
}
