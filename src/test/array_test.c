#include "array_test.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../types/array.h"
#include "minunit.h"
#include "tsh_test.h"

#define ARRAY_TEST_SIZE 4


static char* array_create_test();
static char* array_size_test();
static char* array_insert_test();
static char* array_remove_test();


extern int tests_run;

static char *(*tests[])(void) =
  {
    array_create_test,
    array_size_test,
    array_insert_test,
    array_remove_test
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
  array *arr = array_create(sizeof(int));
  
  const int size = 128;
  for (int i=0; i < size; i++)
    {
      array_insert_last(arr, &i);
    }

  mu_assert("Invalid array size, should be 128", size == array_size(arr));

  array_free(arr, false);

  return 0;
}


static char* array_insert_test()
{
  array *arr = array_create(sizeof(int));
  
  const int size = 128;
  for (int i=0; i < size; i++)
    {
      array_insert_last(arr, &i);
    }

  mu_assert("Invalid array size, should be 128", size == array_size(arr));
  
  for (int i=0; i < array_size(arr); i++)
    {
      mu_assert("Wrong value after inserting", i == *(int*)array_get(arr, i));
    }

  for (int i=0, j = 1; i < size; i+=2, j+=2)
    {
      array_insert(arr, i, &j);
    }

  mu_assert("Invalid array size", size+size/2 == array_size(arr));

  for (int i=0; i < size; i += 2)
    {
      mu_assert("Wrong value after inserting", i+1 == *(int*)array_get(arr, i));
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
    }
  
  mu_assert("Array should be empty", 0 == array_size(arr));
  
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
      printf("%s\n", results);
    }
  else
    {
      printf("ALL ARRAY TESTS PASSED\n");
    }
  
  printf("array tests run: %d\n\n", tests_run - prec_tests_run);
  
  return (results == 0);
}
