#include "utils_test.h"

#include <stdio.h>

#include "minunit.h"
#include "tsh_test.h"
#include "utils.h"


static char* is_prefix_test();

static char *(*tests[])(void) =
  {
    is_prefix_test
  };


extern int tests_run;


static char* is_prefix_test()
{
  mu_assert("is_prefix(\"Hell\", \"Hello World !\") != 1", is_prefix("Hell", "Hello World !") == 1);
  mu_assert("is_prefix(\"Hello\", \"Hello World !\") != 1", is_prefix("Hello", "Hello World !") == 1);
  mu_assert("is_prefix(\"\", \"Hello World !\") != 1", is_prefix("", "Hello World !") == 1);
  
  mu_assert("is_prefix(\"World\", \"Hello World !\") != 0", is_prefix("World", "Hello World !") == 0);
  mu_assert("is_prefix(\"hello\", \"Hello World !\") != 0", is_prefix("hello", "Hello World !") == 0);
  mu_assert("\"is_prefix(\"Hello World !\", \"Hello\") != 0", is_prefix("Hello World !", "Hello") == 0);
  mu_assert("\"is_prefix(\"Hello World !\", \"\") != 0", is_prefix("Hello World !", "") == 0);
  
  mu_assert("is_prefix(\"Hello World !\", \"Hello World !\") != 2", is_prefix("Hello World !", "Hello World !") == 2);
  mu_assert("is_prefix(\"\", \"\") != 2", is_prefix("", "") == 2);
  
  return 0;
}

static char *all_tests()
{
  for (int i = 0; i < UTILS_TEST_SIZE; i++)
    {
      mu_run_test(tests[i]);
    }
  return 0;
}


int launch_utils_tests()
{
  int prec_tests_run = tests_run;

  char *results = all_tests();
  if (results != 0)
    {
      printf(RED "%s\n" WHITE, results);
    }
  else
    {
      printf(GREEN "ALL UTILS TESTS PASSED\n" WHITE);
    }

  printf("utils tests run: %d\n\n", tests_run - prec_tests_run);

  return (results == 0);
}
