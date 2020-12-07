#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tsh_test.h"
#include "minunit.h"
#include "tar.h"
#include "tar_ls_test.h"

extern int tests_run;

static char *test_tar_ls();
static char *all_tests();


int launch_tar_ls_tests()
{
  int prec_tests_run = tests_run;
  char *results = all_tests();
  if (results != 0) {
    printf(RED "%s\n" WHITE, results);
  }
  else {
    printf(GREEN "ALL TAR LS TESTS PASSED\n" WHITE);
  }
  printf("tar ls tests run: %d\n\n", tests_run - prec_tests_run);
  return (results == 0);
}

static char *all_tests()
{
  before();
  mu_run_test(test_tar_ls);
  return 0;
}

static char *test_tar_ls(){
  int tmp, size = 0;
  char *test[] =
  {"dir1/", "dir1/subdir/", "dir1/subdir/subsubdir/",
  "dir1/subdir/subsubdir/hello", "dir1/tata", "man_dir/", "man_dir/man",
  "man_dir/open2", "man_dir/tar", "titi",
  "titi_link", "toto", "dir2/fic1", "dir2/fic2", "access/no",
  "access/x", "access/no_x_dir/", "access/no_x_dir/a"};
  struct posix_header *a_tester = tar_ls("/tmp/tsh_test/test.tar", &size);
  for(int i = 0; i < size; i++) {
    tmp = 0;
    for(int j = 0; j < size; j++)
      mu_assert("Error, this isn't the good ls", strcmp(test[i], a_tester[j].name) == 0 || tmp++ < size );
  }
  free(a_tester);
  return 0;
}
