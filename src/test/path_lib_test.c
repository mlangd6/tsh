#include "minunit.h"
#include "path_lib.h"
#include "tsh_test.h"
#include <stdio.h>
#include <string.h>

#define PATH_LIB_TEST_SIZE 1
static char *split_tar_abs_path_test();

extern int tests_run;
static char *(*tests[])(void) = {split_tar_abs_path_test};


static char *split_tar_abs_path_test() {
  char *root = "/";
  char *tmp = "/tmp";
  char *tmp_bis = "/tmp/";
  char *root_tar = "/tmp/tsh_test/test.tar";
  char *root_tar_bis = "/tmp/tsh_test/test.tar/";
  char *sub_dir_tar = "/tmp/tsh_test/test.tar/man";
  char *sub_dir_tar_bis = "/tmp/tsh_test/test.tar/man/";
  char *test_error = "relative";
  mu_assert("ERROR0", split_tar_abs_path(test_error) == NULL);
  mu_assert("ERROR1", split_tar_abs_path(root)[0] == '\0');
  mu_assert("ERROR2", split_tar_abs_path(tmp)[0] == '\0');
  mu_assert("ERROR3", split_tar_abs_path(tmp_bis)[0] == '\0');
  mu_assert("ERROR4", split_tar_abs_path(root_tar)[0] == '\0');
  mu_assert("ERROR7",  strcmp(split_tar_abs_path(root_tar_bis), "") == 0);
  mu_assert("ERROR5", strcmp(split_tar_abs_path(sub_dir_tar), "man")  == 0);
  mu_assert("ERROR6",  strcmp(split_tar_abs_path(sub_dir_tar_bis), "man/") == 0);
  return 0;
}

static char *all_tests() {
  for (int i = 0; i < PATH_LIB_TEST_SIZE; i++) {
    before();
    mu_run_test(tests[i]);
  }
  return 0;
}

int launch_path_lib_tests() {
  int prec_tests_run = tests_run;
  char *results = all_tests();
  if (results != 0) {
    printf("%s\n", results);
  }
  else {
    printf("ALL PATH_LIB TESTS PASSED\n");
  }
  printf("path_lib tests run: %d\n\n", tests_run - prec_tests_run);

  return (results == 0);
}
