#include "tsh_test.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "minunit.h"

#include "path_lib_test.h"
#include "parse_line_test.h"
#include "list_test.h"
#include "stack_test.h"
#include "tar_add_test.h"
#include "tar_access_test.h"
#include "array_test.h"
#include "tar_ls_test.h"
#include "tar_rm_test.h"
#include "tar_cp_mv_test.h"


int tests_run;

static char *tests_names[] = {
  "tar_rm",
  "tar_ls",
  "tar_access",
  "tar_add",
  "tar_cp_mv",
  "path_lib",
  "parse_line",
  "list",
  "stack",
  "array"
};

static int (*launch_tests[])(void) = {
  launch_tar_rm_tests,
  launch_tar_ls_tests,
  launch_tar_access_tests,
  launch_tar_add_tests,
  launch_tar_cp_mv_tests,
  launch_path_lib_tests,
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

static void print_arguments()
{
  printf("Possible arguments are: \n");
  for (int i = 0; i < NB_TESTS; i++)
  {
    printf("%s\n", tests_names[i]);
  }
  exit(EXIT_SUCCESS);
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
      if (index < 0)
      {
        if (!strcmp(argv[i], "--help"))
          print_arguments();
        else
          printf(RED "Unknown argument %s, launch with option --help to see possible arguments\n\n"
            WHITE, argv[i]);

      }
      else ret = launch_tests[index]() && ret;
    }
  }

  if (ret)
    printf(GREEN "ALL TESTS PASSED\n" WHITE);

  printf("Total tests run: %d\n", tests_run);

  return (ret == 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
