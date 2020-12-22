#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minunit.h"
#include "path_lib.h"
#include "tsh_test.h"
#include "path_lib_test.h"


static char *split_tar_abs_path_test();
static char *reduce_abs_path_root_test();
static char *reduce_abs_path_tar_test();
static char *reduce_abs_path_titi_test();
static char *reduce_abs_path_dir_test();
static char *reduce_abs_path_non_existing_file();
static char *type_of_file_test();


extern int tests_run;

static char *(*tests[])(void) = {
  split_tar_abs_path_test,
  reduce_abs_path_root_test,
  reduce_abs_path_tar_test,
  reduce_abs_path_titi_test,
  reduce_abs_path_dir_test,
  reduce_abs_path_non_existing_file,
  type_of_file_test
};


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
    printf(RED "%s\n" WHITE, results);
  }
  else {
    printf(GREEN "ALL PATH_LIB TESTS PASSED\n" WHITE);
  }
  printf("path_lib tests run: %d\n\n", tests_run - prec_tests_run);

  return (results == 0);
}


static char *split_tar_abs_path_test() {
  mu_assert("split_tar_abs_path: error: should return NULL with NULL", split_tar_abs_path(NULL) == NULL);

  char test_error[] = "..";
  mu_assert("split_tar_abs_path: error: should return NULL with \"relative\"", split_tar_abs_path(test_error) == NULL);

  char root[] = "/";
  mu_assert("split_tar_abs_path: error: should return NULL with \"/\"", split_tar_abs_path(root) == NULL);

  char tmp[] = "/tmp";
  mu_assert("split_tar_abs_path: error: should return NULL with \"/tmp\"", split_tar_abs_path(tmp) == NULL);

  char tmp_bis[] = "/tmp/";
  mu_assert("split_tar_abs_path: error: should return NULL with \"/tmp/\"", split_tar_abs_path(tmp_bis) == NULL);

  char root_tar[] = "/tmp/tsh_test/test.tar";
  mu_assert("split_tar_abs_path: error: should return \\0 with \"/tmp/tsh_test/test.tar\"", split_tar_abs_path(root_tar)[0] == '\0');

  char root_tar_bis[] = "/tmp/tsh_test/test.tar/";
  mu_assert("split_tar_abs_path: error: should return \\0 with \"/tmp/tsh_test/test.tar/\"", split_tar_abs_path(root_tar_bis)[0] == '\0');

  char sub_dir_tar[] = "/tmp/tsh_test/test.tar/man";
  mu_assert("split_tar_abs_path: error: should return \"man\" with \"/tmp/tsh_test/test.tar/man\"", strcmp(split_tar_abs_path(sub_dir_tar), "man")  == 0);

  char sub_dir_tar_bis[] = "/tmp/tsh_test/test.tar/man/";
  mu_assert("split_tar_abs_path: error: should return \"man/\" with \"/tmp/tsh_test/test.tar/man/", strcmp(split_tar_abs_path(sub_dir_tar_bis), "man/") == 0);

  return 0;
}

static char *reduce_abs_path_root_test() {
  char root[PATH_MAX];

  const char *to_test[] =
    { "/",
      "/.",
      "/..",
      "/////",
      "///..///../..///.././.././..///.././//.",
      "/././///tmp/..////.//",
      "//tmp//./.././tmp/.///tsh_test/./././///..//.//..////.///./",
      "///tmp/.//.//tsh_test/.././././//../",
      "/tmp/tsh_test/test.tar/man_dir/../dir1/subdir/subsubdir/../../../../../../../../.." };

  for (int i=0; i < 9; i++)
    {
      mu_assert("reduce_abs_path: error: Should return \"/\"", strcmp(reduce_abs_path(to_test[i], root), "/") == 0);
    }

  return 0;
}


static char *reduce_abs_path_tar_test()
{
  char path[PATH_MAX];

  // without slash
  const char *to_test1[] =
    { "/tmp/tsh_test/test.tar",
      "/tmp/././////////////tsh_test/.././../tmp/tsh_test/./test.tar",
      "/..//tmp//./././tsh_test////test.tar/../test.tar"};

  for (int i=0; i < 3; i++)
    {
      mu_assert("reduce_abs_path: error: Should return \"/tmp/tsh_test/test.tar\"", strcmp(reduce_abs_path(to_test1[i], path), "/tmp/tsh_test/test.tar") == 0);
    }

  //with slash
  const char *to_test2[] =
    { "/tmp/tsh_test/test.tar/",
      "/tmp/././////////////tsh_test/.././../tmp/tsh_test/./test.tar///",
      "/..//tmp//./././tsh_test////test.tar/../test.tar///",
      "////..//tmp/tsh_test//test.tar/dir1/subdir//../../man_dir/../dir1/subdir/subsubdir/../../..//" };

  for (int i=0; i < 4; i++)
    {
      mu_assert("reduce_abs_path: error: Should return \"/tmp/tsh_test/test.tar/\"", strcmp(reduce_abs_path(to_test2[i], path), "/tmp/tsh_test/test.tar/") == 0);
    }

  return 0;
}


static char *reduce_abs_path_titi_test() {
  char path[PATH_MAX];

  const char *to_test[] =
    { "/tmp/tsh_test/test.tar/titi",
      "/tmp/././////////////tsh_test/.././../tmp/tsh_test/./test.tar/titi",
      "/..//tmp//./././tsh_test////test.tar/../test.tar/././/dir1/../titi",
      "/..//.././tmp/tsh_test/test.tar/dir1//../man_dir/./../dir1/subdir//subsubdir/./..//../..//./titi" };

  for (int i=0; i < 4; i++)
    {
      mu_assert("reduce_abs_path: error: Should return \"/tmp/tsh_test/test.tar/titi\"", strcmp(reduce_abs_path(to_test[i], path), "/tmp/tsh_test/test.tar/titi") == 0);
    }

  mu_assert("reduce_abs_path should return \"/tmp/tsh_test/test.tar/titi/\"", !strcmp(reduce_abs_path("/tmp/tsh_test/test.tar/titi/////", path), "/tmp/tsh_test/test.tar/titi/"));
  mu_assert("reduce_abs_path shouldn't change path", !strcmp(reduce_abs_path("/tmp/tsh_test/test.tar/titi/", path), "/tmp/tsh_test/test.tar/titi/"));

  return 0;
}

static char *reduce_abs_path_non_existing_file()
{
  char path[PATH_MAX];
  const char *to_test[] =
  {
    "/tmp/tsh_test/test.tar/file",
    "/tmp/.//////////////////.././tmp/././././/tsh_test/.//test.tar/../test.tar/file",
    "/tmp/.////////tsh_test/../tsh_test/test.tar/dir1/../access/./////../file"
  };
  for (int i = 0; i < 3; i++)
  {
    mu_assert("reduce_abs_path: error: should return \"/tmp/tsh_test/test.tar/file\"", !strcmp(reduce_abs_path(to_test[i], path), "/tmp/tsh_test/test.tar/file"));
  }
  mu_assert("reduce_abs_path: error: should return \"/tmp/tsh_test/file\"", !strcmp(reduce_abs_path("/tmp/./tsh_test/file", path), "/tmp/tsh_test/file"));
  mu_assert("reduce_abs_path: error: should return \"/tmp/tsh_test/test.tar/dir1/subdir/subsubdir/file\"", !strcmp(reduce_abs_path("/tmp/tsh_test/test.tar/dir1/.///////./././///subdir/../././../dir1/subdir/subsubdir/././///file", path), "/tmp/tsh_test/test.tar/dir1/subdir/subsubdir/file"));
  return 0;
}


static char *reduce_abs_path_dir_test()
{
  char path[PATH_MAX];

  const char *to_test[] =
    { "/tmp/tsh_test/test.tar/dir1/subdir/",
      "//tmp/tsh_test//../tsh_test/test.tar/dir1//./././/../../test.tar/dir1/subdir/./." };

  for (int i=0; i < 2; i++)
    {
      mu_assert("reduce_abs_path: error: Should return \"/tmp/tsh_test/test.tar/dir1/subdir/\"", strcmp(reduce_abs_path(to_test[i], path), "/tmp/tsh_test/test.tar/dir1/subdir/") == 0);
    }

  mu_assert("reduce_abs_path shouldn't change path", !strcmp(reduce_abs_path("/tmp/tsh_test/test.tar/dir1/subdir", path), "/tmp/tsh_test/test.tar/dir1/subdir"));


  return 0;
}

static char *type_of_file_test()
{
  char cmd[1024];
  char *a = "a";
  char *tar = "test.tar";
  sprintf(cmd, "cd %s; touch %s; touch dir2; tar -rf %s %s dir2; rm %s; mkdir %s; tar -rf %s %s", TEST_DIR, a, tar, a, a, a, tar, a);
  system(cmd);
  mu_assert("type_of_file with dir_priority should return DIR with a", type_of_file(TAR_TEST, a, true) == DIR);
  mu_assert("type_of_file without dir_priority should return REG with a", type_of_file(TAR_TEST, a, false) == REG);
  mu_assert("type_of_file with dir_priority should return DIR with dir2", type_of_file(TAR_TEST, "dir2", true) == DIR);
  mu_assert("type_of_file without dir_priority should return REG with dir2", type_of_file(TAR_TEST, "dir2", false) == REG);
  mu_assert("type_of_file without dir_priority should return DIR with dir2/", type_of_file(TAR_TEST, "dir2/", true) == DIR);
  mu_assert("type_of_file with dir_priority should return DIR with dir1", type_of_file(TAR_TEST, "dir1", true) == DIR);
  mu_assert("type_of_file without dir_priority should return DIR with dir1", type_of_file(TAR_TEST, "dir1", false) == DIR);
  mu_assert("type_of_file without dir_priority should return DIR with toto", type_of_file(TAR_TEST, "toto", false) == REG);
  mu_assert("type_of_file with dir_priority should return DIR with toto", type_of_file(TAR_TEST, "toto", true) == REG);
  mu_assert("type_of_file should always return NONE with toto/", type_of_file(TAR_TEST, "toto/", true) == NONE && type_of_file(TAR_TEST, "toto/", false) == NONE);
  return 0;
}
