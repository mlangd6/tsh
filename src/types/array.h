/* array.h : Functions prototypes for array manipulation */
#ifndef ARRAY_H
#define ARRAY_H

#include <stdbool.h>
#include <stddef.h>

typedef struct array array;

array *array_create (size_t elem_size);

void array_free (array *arr, bool full);


int array_size (array *arr);


void *array_set (array *arr, size_t i, void *val);

void array_insert (array *arr, size_t i, void *val);

void array_insert_first (array *arr, void *val);

void array_insert_last (array *arr, void *val);



void *array_get (array *arr, size_t i);

void *array_remove (array *arr, size_t i);

void *array_remove_first (array *arr);

void *array_remove_last (array *arr);


void array_sort (array *arr, int (*comp)(const void *, const void *));

#endif
