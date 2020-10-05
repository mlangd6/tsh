#include "minunit.h"
#include "tar.h"
#include "tsh_test.h"
#include <stdio.h>
#include <string.h>

static char *test_tar_ls() {
  before();
  char **a_tester = tar_ls("/tmp/tsh_test/test.tar");
  char *test[12] = {"dir1/", "dir1/subdir/", "dir1/subdir/subsubdir/", "dir1/subdir/subsubdir/hello", "dir1/tata", "man_dir/", "man_dir/man", "man_dir/open2", "man_dir/tar", "titi", "titi_link", "toto"};
  for(int i = 0; i < 12; i++){
    mu_assert("Error, this isn't the good ls", strcmp(test[i], a_tester[i]) == 0);
  }
  return 0;
}

static char *all_tests(){
  m_run(test_tar_ls);
  return 0;
}

static int tmp(int argc, char const *argv[]) {
  char *result = all_tests();
  if(result != 0)
    printf("%s\n", result);
  else
    printf("ALL TESTS PASSED\n");
  printf("Tests run: %d\n", tests_run);
  return result != 0;
}
