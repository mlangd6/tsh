#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../types/stack.h"
#include "minunit.h"
#include "tsh_test.h"

#define STACK_TEST_SIZE 3


static char* stack_create_test();
static char* stack_size_test();
static char* stack_push_pop_test();

extern int tests_run;

static char *(*tests[])(void) =
  {
    stack_create_test,
    stack_size_test,
    stack_push_pop_test
  };


static char* stack_create_test()
{
  stack_t *stack = stack_create();

  mu_assert("Stack should be empty", 1 == stack_is_empty(stack));
  mu_assert("Invalid stack size, should be 0", 0 == stack_size(stack));

  stack_free(stack, false);
  
  return 0;
}
  

static char* stack_size_test()
{
  stack_t *stack = stack_create();
  
  const int size = 15;
  for (int i=0; i < size; i++)
    {
      stack_push(stack, NULL);
    }

  mu_assert("Invalid stack size, should be 15", size == stack_size(stack));

  stack_free(stack, false);

  return 0;
}


static char* stack_push_pop_test()
{
  stack_t *stack = stack_create();  
  const int size = 15;
  int *pi;

  
  for (int i=0; i < size; i++)
    {
      pi = malloc(sizeof(int));
      *pi = i;
      stack_push(stack, pi);
    }

  mu_assert("Invalid stack size, should be 15", size == stack_size(stack));
  
  for (int i=size-1; i >= 0; i--)
    {
      pi = (int*)stack_pop(stack);
      mu_assert("Incorrect value", i == *pi);
      free(pi);
    }

  
  mu_assert("Stack should be empty", 1 == stack_is_empty(stack));
  stack_free(stack, false);

  return 0;
}


static char *all_tests()
{
  for (int i = 0; i < STACK_TEST_SIZE; i++)
    {
      before();
      mu_run_test(tests[i]);
    }
  return 0;
}


int launch_stack_tests()
{
  int prec_tests_run = tests_run;

  char *results = all_tests();
  if (results != 0)
    {
      printf("%s\n", results);
    }
  else
    {
      printf("ALL STACK TESTS PASSED\n");
    }
  
  printf("stack tests run: %d\n\n", tests_run - prec_tests_run);
  
  return (results == 0);
}

