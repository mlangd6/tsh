#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "tsh_test.h"
#include "minunit.h"
#include "tar.h"
#include "tar_ls_test.h"
#include "array.h"

extern int tests_run;

static char *all_tests();

static char *tar_ls_test();
static char *tar_ls_all_test();
static char *tar_ls_dir_root_test();
static char *tar_ls_dir_man_dir_test();
static char *tar_ls_dir_dir1_rec_test();

static char *(*tests[])(void) = {
  tar_ls_test,
  tar_ls_all_test,
  tar_ls_dir_root_test,
  tar_ls_dir_man_dir_test,
  tar_ls_dir_dir1_rec_test
};

int launch_tar_ls_tests()
{
  int prec_tests_run = tests_run;
  char *results = all_tests();
  
  if (results != 0)
    {
      printf(RED "%s\n" WHITE, results);
    }
  else
    {
      printf(GREEN "ALL TAR LS TESTS PASSED\n" WHITE);
    }
  printf("tar ls tests run: %d\n\n", tests_run - prec_tests_run);
  return (results == 0);
}

static char *all_tests()
{
  for (int i = 0; i < TAR_LS_TEST_SIZE; i++)
  {
    before();
    mu_run_test(tests[i]);
  }

  return 0;
}

static char *tar_ls_test()
{
  int tmp, size = 0;
  
  char *test[] =
    {
      "dir1/",
      "dir1/subdir/",
      "dir1/subdir/subsubdir/",
      "dir1/subdir/subsubdir/hello",
      "dir1/tata",
      "man_dir/",
      "man_dir/man",
      "man_dir/open2",
      "man_dir/tar",
      "titi",
      "titi_link",
      "toto",
      "dir2/fic1",
      "dir2/fic2",
      "access/no",
      "access/x",
      "access/no_x_dir/",
      "access/no_x_dir/a"
    };
  
  struct posix_header *a_tester = tar_ls("/tmp/tsh_test/test.tar", &size);
  for(int i = 0; i < size; i++) {
    tmp = 0;
    for(int j = 0; j < size; j++)
      mu_assert("Error, this isn't the good ls", strcmp(test[i], a_tester[j].name) == 0 || tmp++ < size );
  }
  free(a_tester);
  return 0;
}


static int sort_header_name(const void *lhs, const void *rhs)
{
  return strcmp(((tar_file*)lhs)->header.name, ((tar_file*)rhs)->header.name);
}
  

static char *tar_ls_all_test()
{
  int tar_fd = open("/tmp/tsh_test/test.tar", O_RDONLY);  
  array *arr = tar_ls_all(tar_fd);

  int n = array_size(arr);
  mu_assert("There should be 18 files in test.tar", 18 == n);
  
  char *test[] =
    {
      "access/no",
      "access/no_x_dir/",
      "access/no_x_dir/a",
      "access/x",
      "dir1/",
      "dir1/subdir/",
      "dir1/subdir/subsubdir/",
      "dir1/subdir/subsubdir/hello",
      "dir1/tata",
      "dir2/fic1",
      "dir2/fic2",
      "man_dir/",
      "man_dir/man",
      "man_dir/open2",
      "man_dir/tar",
      "titi",
      "titi_link",
      "toto"
    };

  array_sort(arr, sort_header_name);

  tar_file *tf;
  for (int i=0; i < n; i++)
    {
      tf = (tar_file*)array_get(arr, i);
      mu_assert("Wrong file descriptor", tf->tar_fd == tar_fd);
      mu_assert("Invalid ls", !strcmp(tf->header.name, test[i]));
      free(tf);
    }
  
  array_free(arr, false);
  close(tar_fd);
  
  return 0;
}

static char *tar_ls_dir_root_test()
{
  int tar_fd = open("/tmp/tsh_test/test.tar", O_RDONLY);  
  array *arr = tar_ls_dir(tar_fd, "", false);

  int n = array_size(arr);
  mu_assert("There should be 5 files at root in test.tar", n == 5);
  
  char *test[] =
    {      
      "dir1/",
      "man_dir/",
      "titi",
      "titi_link",
      "toto"
    };

  array_sort(arr, sort_header_name);

  tar_file *tf;
  for (int i=0; i < n; i++)
    {
      tf = (tar_file*)array_get(arr, i);
      mu_assert("Wrong file descriptor", tf->tar_fd == tar_fd);
      mu_assert("Invalid ls", !strcmp(tf->header.name, test[i]));
      free(tf);
    }
  
  array_free(arr, false);
  close(tar_fd);
  
  return 0;
}

static char *tar_ls_dir_man_dir_test()
{
  int tar_fd = open("/tmp/tsh_test/test.tar", O_RDONLY);  
  array *arr = tar_ls_dir(tar_fd, "man_dir/", false);

  int n = array_size(arr);
  mu_assert("There should be 3 files in man_dir/ in test.tar", n == 3);
  
  char *test[] =
    {
      "man_dir/man",
      "man_dir/open2",
      "man_dir/tar"
    };

  array_sort(arr, sort_header_name);

  tar_file *tf;
  for (int i=0; i < n; i++)
    {
      tf = (tar_file*)array_get(arr, i);
      mu_assert("Wrong file descriptor", tf->tar_fd == tar_fd);
      mu_assert("Invalid ls", !strcmp(tf->header.name, test[i]));
      free(tf);
    }
  
  array_free(arr, false);
  close(tar_fd);
  
  return 0;
}

static char *tar_ls_dir_dir1_rec_test()
{
  int tar_fd = open("/tmp/tsh_test/test.tar", O_RDONLY);  
  array *arr = tar_ls_dir(tar_fd, "dir1/", true);

  int n = array_size(arr);
  mu_assert("There should be 4 files in dir1/ subdirectories in test.tar", n == 4);
  
  char *test[] =
    {
      "dir1/subdir/",
      "dir1/subdir/subsubdir/",
      "dir1/subdir/subsubdir/hello",
      "dir1/tata"
    };

  array_sort(arr, sort_header_name);

  tar_file *tf;
  for (int i=0; i < n; i++)
    {
      tf = (tar_file*)array_get(arr, i);
      mu_assert("Wrong file descriptor", tf->tar_fd == tar_fd);
      mu_assert("Invalid ls", !strcmp(tf->header.name, test[i]));
      free(tf);
    }
  
  array_free(arr, false);
  close(tar_fd);

  return 0;
}
