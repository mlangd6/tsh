#include <stdlib.h>
#include <stdio.h>

#include "minunit.h"
#include "tsh_test.h"

#include "path_lib_test.h"
#include "parse_line_test.h"
#include "list_test.h"
#include "stack_test.h"
#include "tar_test.h"


int tests_run;

void before()
{
  system("./create_test_tar.sh");
}

int main(int argc, char const *argv[])
{
  int ret = 1;

  ret = launch_path_lib_tests() && ret;
  ret = launch_tar_tests() && ret;
  ret = launch_parse_line_tests() && ret;
  ret = launch_list_tests() && ret;
  ret = launch_stack_tests() && ret;

  if (ret)
    printf("ALL TESTS PASSED\n");

  printf("Total tests run: %d\n", tests_run);

  return (ret == 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
