#include <stdlib.h>
#include <stdio.h>

#include "tsh_test.h"
#include "minunit.h"
#include "tar_rm_test.h"
#include "tar.h"

static char *all_tests();
static char *tar_rm_file_test();
static char *tar_rm_dir_test();

extern int tests_run;

static char *(*tests[])(void) = {
  tar_rm_file_test,
  tar_rm_dir_test
};

int launch_tar_rm_tests()
{
  int prec_tests_run = tests_run;
  char *results = all_tests();
  if (results != 0) {
    printf(RED "%s\n" WHITE, results);
  }
  else {
    printf(GREEN "ALL TAR RM TESTS PASSED\n" WHITE);
  }
  printf("tar rm tests run: %d\n\n", tests_run - prec_tests_run);
  return (results == 0);
}

static char *all_tests()
{
  for (int i = 0; i < TAR_RM_TEST_SIZE; i++)
  {
    before();
    mu_run_test(tests[i]);
  }
  return 0;
}

static char *tar_rm_file_test()
{
  mu_assert("Couldn't remove man_dir/open2", tar_rm("/tmp/tsh_test/test.tar", "man_dir/open2") == 0);
  mu_assert("Error tar_rm corrupted the tar", is_tar("/tmp/tsh_test/test.tar") == 1);

  mu_assert("tar_rm(\"/tmp/tsh_test/test.tar\", \"man_dir/open2\") != -2", tar_rm("/tmp/tsh_test/test.tar", "man_dir/open2") == -2);

  return 0;
}

static char *tar_rm_dir_test()
{
  mu_assert("Couldn't remove dir1/", tar_rm("/tmp/tsh_test/test.tar","dir1/") == 0);
  mu_assert("Error tar_rm corrupted the tar", is_tar("/tmp/tsh_test/test.tar") == 1);

  return 0;
}
