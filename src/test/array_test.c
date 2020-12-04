/* array_test.c : Tests for array data types */
#include "array_test.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "minunit.h"
#include "tsh_test.h"

#define ARRAY_TEST_SIZE 5


static char* array_create_test();
static char* array_size_test();
static char* array_insert_test();
static char* array_remove_test();
static char* array_sort_test();

extern int tests_run;

static char *(*tests[])(void) =
  {
    array_create_test,
    array_size_test,
    array_insert_test,
    array_remove_test,
    array_sort_test
  };


static char* array_create_test()
{
  array *arr = array_create(sizeof(char));

  mu_assert("Invalid array size, should be 0", 0 == array_size(arr));

  array_free(arr, false);

  return 0;
}


static char* array_size_test()
{
  array *arr = array_create(sizeof(long long));
  mu_assert("Array should be empty after creation", 0 == array_size(arr));

  const int size = 2020;
  for (int i=0; i < size; i++)
    {
      array_insert_last(arr, &i);
    }

  mu_assert("Invalid array size, should be 2020", size == array_size(arr));

  array_free(arr, false);

  return 0;
}


static char* array_insert_test()
{
  array *arr = array_create(sizeof(int));

  const int size = 4291;
  for (int i=0; i < size; i++)
    {
      array_insert_last(arr, &i);
    }

  mu_assert("Invalid array size, should be 4291", size == array_size(arr));

  int *pi;
  for (int i=0; i < array_size(arr); i++)
    {
      pi = (int*)array_get(arr, i);
      mu_assert("Wrong value after inserting", i == *pi);
      free(pi);
    }

  for (int i=0, j = 1; i < size; i+=2, j+=2)
    {
      array_insert(arr, i, &j);
    }

  mu_assert("Invalid array size", size + size/2 + size%2 == array_size(arr));

  for (int i=0; i < size; i += 2)
    {
      pi = (int*)array_get(arr, i);
      mu_assert("Wrong value after inserting", i+1 == *pi);
      free(pi);
    }

  array_free(arr, false);

  return 0;
}

static char* array_remove_test()
{
  array *arr = array_create(sizeof(char));

  for (char c='a'; c <= 'z'; c++)
    {
      array_insert_first(arr, &c);
    }

  mu_assert("Invalid array size, should be 26", 26 == array_size(arr));

  char *pc;
  for (char c='a'; c <= 'z'; c++)
    {
      pc = (char*)array_remove_last(arr);
      mu_assert("Wrong value after inserting", c == *pc);
      free(pc);
    }

  mu_assert("Array should be empty", 0 == array_size(arr));

  array_free(arr, false);

  return 0;
}

static int cmp(const void *l, const void *r)
{
  return strcmp((char*)l, (char*)r);
}

static char* array_sort_test()
{
  array *arr = array_create(sizeof(char*));
  char *name[] =
    {
      "dir/",
      "dir/tata",
      "dir/tutu",
      "dir/tata_dir/",
      "dir/tata_dir/tutu",
      "dir/z/",
      "aaa"
    };
  const int name_size = 7;

  for (int i=0; i < name_size; i++)
    {
      array_insert_first(arr, name+i);
    }
  mu_assert("Invalid array size after inserting", name_size == array_size(arr));

  array_sort(arr, cmp);
  mu_assert("Invalid array size after sort", name_size == array_size(arr));

  char** ppc;
  for (int i=0; i < name_size; i++)
    {
      ppc = (char**)array_get(arr, i);
      mu_assert("Sort didn't work", 0 == strcmp(name[i], *ppc));
      free(ppc);
    }

  array_free(arr, false);

  return 0;
}

static char *all_tests()
{
  for (int i = 0; i < ARRAY_TEST_SIZE; i++)
    {
      mu_run_test(tests[i]);
    }
  return 0;
}


int launch_array_tests()
{
  int prec_tests_run = tests_run;

  char *results = all_tests();
  if (results != 0)
    {
      printf(RED "%s\n" WHITE, results);
    }
  else
    {
      printf(GREEN "ALL ARRAY TESTS PASSED\n" WHITE);
    }

  printf("array tests run: %d\n\n", tests_run - prec_tests_run);

  return (results == 0);
}
