#include "minunit.h"
#include "tsh_test.h"
#include "tar.h"
#include <stdio.h>
#define TAR_TEST_SIZE 1


static char *tar_add_file_test();
int tests_run = 0;

static char *(*tests[])(void) = {tar_add_file_test};

static char *tar_add_file_test() {
  
  return 0;
}
static char *all_tests() {
  for (int i = 0; i < TAR_TEST_SIZE; i++) {
    before();
    mu_run_test(tests[i]);
  }
}

int launch_tests() {
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
