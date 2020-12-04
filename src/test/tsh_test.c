#include "tsh_test.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "minunit.h"

#include "path_lib_test.h"
#include "parse_line_test.h"
#include "list_test.h"
#include "stack_test.h"
#include "tar_test.h"
#include "array_test.h"


int tests_run;

static char *tests_names[] = {
  "path_lib",
  "tar",
  "parse_line",
  "list",
  "stack",
  "array"
};

static int (*launch_tests[])(void) = {
  launch_path_lib_tests,
  launch_tar_tests,
  launch_parse_line_tests,
  launch_list_tests,
  launch_stack_tests,
  launch_array_tests
};

static int index_of(char *s)
{
  for (int i = 0; i < NB_TESTS; i++)
  {
    if (!strcmp(tests_names[i], s)) return i;
  }
  return -1;
}

void before()
{
  system("./create_test_tar.sh");
}

int main(int argc, char **argv)
{
  int ret = 1;
  int i = 1, j = 0;
  for ( ; i < argc || (argc == 1 && j < NB_TESTS) ; i++, j++)
  {
    if (argc == 1) ret = launch_tests[j]() && ret;
    else
    {
      int index = index_of(argv[i]);
      if (index < 0) printf(RED "Unknown argument %s\n\n" WHITE, argv[i]);
      else ret = launch_tests[index]() && ret;
    }
  }

  if (ret)
    printf(GREEN "ALL TESTS PASSED\n" WHITE);

  printf("Total tests run: %d\n", tests_run);

  return (ret == 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
