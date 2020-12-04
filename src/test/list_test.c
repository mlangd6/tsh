/* list_test.c : Tests for list data types */
#include "list_test.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "minunit.h"
#include "tsh_test.h"

#define LIST_TEST_SIZE 4


static char* list_create_test();
static char* list_size_test();
static char* list_ins_rem_last_test();
static char* list_ins_rem_first_test();

extern int tests_run;

static char *(*tests[])(void) =
  {
    list_create_test,
    list_size_test,
    list_ins_rem_last_test,
    list_ins_rem_first_test
  };


static char* list_create_test()
{
  list_t *list = list_create();

  mu_assert("List should be empty", 1 == list_is_empty(list));
  mu_assert("Invalid list size, should be 0", 0 == list_size(list));

  list_free(list, false);

  return 0;
}


static char* list_size_test()
{
  list_t *list = list_create();

  const int size = 15;
  for (int i=0; i < size; i++)
    {
      list_insert_last(list, NULL);
    }

  mu_assert("Invalid list size, should be 15", size == list_size(list));

  list_free(list, false);

  return 0;
}


static char* list_ins_rem_last_test()
{
  list_t *list = list_create();
  const int size = 15;
  int *pi;


  for (int i=0; i < size; i++)
    {
      pi = malloc(sizeof(int));
      *pi = i;
      list_insert_last(list, pi);
    }

  for (int i=size-1; i >= 0; i--)
    {
      pi = (int*)list_remove_last(list);
      mu_assert("Incorrect value", i == *pi);
      free(pi);
    }


  mu_assert("List should be empty", 1 == list_is_empty(list));
  list_free(list, false);

  return 0;
}


static char* list_ins_rem_first_test()
{
  list_t *list = list_create();
  const int size = 15;
  int *pi;


  for (int i=0; i < size; i++)
    {
      pi = malloc(sizeof(int));
      *pi = i;
      list_insert_first(list, pi);
    }

  for (int i=size-1; i >= 0; i--)
    {
      pi = (int*)list_remove_first(list);
      mu_assert("Incorrect value", i == *pi);
      free(pi);
    }

  mu_assert("List should be empty", 1 == list_is_empty(list));
  list_free(list, false);

  return 0;
}

static char *all_tests()
{
  for (int i = 0; i < LIST_TEST_SIZE; i++)
    {
      mu_run_test(tests[i]);
    }
  return 0;
}


int launch_list_tests()
{
  int prec_tests_run = tests_run;

  char *results = all_tests();
  if (results != 0)
    {
      printf(RED "%s\n" WHITE, results);
    }
  else
    {
      printf(GREEN "ALL LIST TESTS PASSED\n" WHITE);
    }

  printf("list tests run: %d\n\n", tests_run - prec_tests_run);

  return (results == 0);
}
