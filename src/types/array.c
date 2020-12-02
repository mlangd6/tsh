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

  void *data;
};



static void array_resize (array *arr, size_t new_capacity)
{
  arr->data = realloc(arr->data, arr->elem_size * new_capacity);
  assert(arr->data);

  arr->capacity = new_capacity;
}



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



int array_size (array *arr)
{
  return arr ? arr->size : -1;
}



void array_set (array *arr, size_t i, void *val)
{
  if (!arr || arr->size <= i)
    return;
  
  memmove (arr->data + i*arr->elem_size, val, arr->elem_size);
}

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

void array_insert_first (array *arr, void *val)
{
  array_insert (arr, 0, val);
}

void array_insert_last (array *arr, void *val)
{
  array_insert (arr, arr->size, val);
}



void *array_get (array *arr, size_t i)
{
  return (!arr || arr->size <= i) ? NULL : arr->data + i*arr->elem_size;
}

void *array_remove (array *arr, size_t i)
{
  if (!arr || arr->size <= i)
    return NULL;
  
  void *ret = arr->data + i;
  
  memmove (arr->data + i*arr->elem_size, arr->data + (1+i)*arr->elem_size, (arr->size - (i+1))*arr->elem_size);

  arr->size--;
  
  if (arr->capacity >= 4 * arr->size && arr->capacity > ARRAY_INITIAL_CAPACITY)
    array_resize(arr, arr->capacity / 2);

  return ret;
}

void *array_remove_first (array *arr)
{
  return array_remove (arr, 0);
}

void *array_remove_last (array *arr)
{
  return array_remove (arr, arr->size-1);
}



void array_sort (array *arr, int (*comp)(const void *, const void *))
{
  qsort (arr->data, arr->size, arr->elem_size, comp);
}
