#include "minunit.h"
#include "tsh_test.h"
#include "tar.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define TAR_TEST_SIZE 2
#define TAR_ADD_TEST_SIZE_BUF 700

static char *tar_add_file_test();
static char *test_tar_ls();
int tests_run = 0;


static char *(*tests[])(void) = {tar_add_file_test, test_tar_ls};

static char *tar_add_file_test() {
  return 0;
}

static char *test_tar_ls() {
  int tmp;
  char **a_tester = tar_ls("/tmp/tsh_test/test.tar");
  char *test[12] = {"dir1/", "dir1/subdir/", "dir1/subdir/subsubdir/", "dir1/subdir/subsubdir/hello", "dir1/tata", "man_dir/", "man_dir/man", "man_dir/open2", "man_dir/tar", "titi", "titi_link", "toto"};
  for(int i = 0; i < 12; i++) {
    tmp = 0;
    for(int j = 0; j < 12; j++)
      mu_assert("Error, this isn't the good ls", strcmp(test[i], a_tester[j]) == 0 || tmp++ < 12 );
  }
  return 0;
}

static char *all_tests() {
  for (int i = 0; i < TAR_TEST_SIZE; i++) {
    before();
    mu_run_test(tests[i]);
  }
  return 0;
}

int launch_tar_tests() {
  char *results = all_tests();
  if (results != 0) {
    printf("%s\n", results);
  }
  else {
    printf("ALL TAR TESTS PASSED\n");
  }
  printf("tar tests run: %d\n", tests_run);

  return results != 0;
}
