#include "minunit.h"
#include "path_lib.h"
#include "tsh_test.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PATH_LIB_TEST_SIZE 3
static char *split_tar_abs_path_test();
static char *reduce_abs_path_root_test();
static char *reduce_abs_path_test();

extern int tests_run;
static char *(*tests[])(void) = {split_tar_abs_path_test, reduce_abs_path_root_test, reduce_abs_path_test};


static char *split_tar_abs_path_test() {
  char root[] = "/";
  char tmp[] = "/tmp";
  char tmp_bis[] = "/tmp/";
  char root_tar[] = "/tmp/tsh_test/test.tar";
  char root_tar_bis[] = "/tmp/tsh_test/test.tar/";
  char sub_dir_tar[] = "/tmp/tsh_test/test.tar/man";
  char sub_dir_tar_bis[] = "/tmp/tsh_test/test.tar/man/";
  char test_error[] = "relative";
  mu_assert("split_tar_abs_path: error: should return NULL with \"relative\"",
	    split_tar_abs_path(test_error) == NULL);
  mu_assert("split_tar_abs_path: error: should return a NULL char with \"/\"",
	    split_tar_abs_path(root)[0] == '\0');
  mu_assert("split_tar_abs_path: error: should return a NULL char with \"/tmp\"",
	    split_tar_abs_path(tmp)[0] == '\0');
  mu_assert("split_tar_abs_path: error: should return a NULL char with \"/tmp/\"",
	    split_tar_abs_path(tmp_bis)[0] == '\0');
  mu_assert("split_tar_abs_path: error: should return a NULL char with \"/tmp/tsh_test/test.tar\"",
	    split_tar_abs_path(root_tar)[0] == '\0');
  mu_assert("split_tar_abs_path: error: should return a NULL char with \"/tmp/tsh_test/test.tar/\"",
	    strcmp(split_tar_abs_path(root_tar_bis), "") == 0);
  mu_assert("split_tar_abs_path: error: should return \"man\" with \"/tmp/tsh_test/test.tar/man\"",
	    strcmp(split_tar_abs_path(sub_dir_tar), "man")  == 0);
  mu_assert("split_tar_abs_path: error: should return \"man/\" with \"/tmp/tsh_test/test.tar/man/",
	    strcmp(split_tar_abs_path(sub_dir_tar_bis), "man/") == 0);
  return 0;
}

static char *reduce_abs_path_root_test() {
  char root1[] = "/";
  char root2[] = "/..";
  char root3[] = "/////";
  char root4[] = "///..///../..///.././.././..///.././//.";
  char root5[] = "/././///tmp/..////.//";
  char root6[] = "//tmp//./.././tmp/.///tsh_test./././///..//.//..////.///./";
  char root7[] = "///tmp/.//.//tsh_test/.././././//..";

  char *res[]  = { reduce_abs_path(root1), reduce_abs_path(root2), reduce_abs_path(root3), reduce_abs_path(root4),
                   reduce_abs_path(root5), reduce_abs_path(root6), reduce_abs_path(root7) };

  for (int i = 0; i < 7; i++)
    {
      mu_assert("reduce_abs_path: error: Should return \"/\"", strcmp(res[i], "/") == 0);
    }

  return 0;
}


static char *reduce_abs_path_test()
{
  char path1[] = "/tmp/././////////////tsh_test/.././../tmp/tsh_test/./test.tar";
  char path2[] = "/tmp/tsh_test/test.tar";
  char path3[] = "/tmp/tsh_test/test.tar/";
  char path4[] = "/..//tmp//./././tsh_test////test.tar/../test.tar";
  char path5[] = "/..//tmp//./././tsh_test////test.tar/../test.tar//////////////";

  char *res[]  = { reduce_abs_path(path1), reduce_abs_path(path2), reduce_abs_path(path3), reduce_abs_path(path4),
                   reduce_abs_path(path5) };


  for (int i = 0; i < 5; i++)
    {
      mu_assert("reduce_abs_path: error: Should return \"/tmp/tsh_test/test.tar\"", strcmp(res[i], "/tmp/tsh_test/test.tar") == 0);
    }
  
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
