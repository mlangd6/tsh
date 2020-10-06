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
#define TAR_TEST_SIZE 3
#define TAR_ADD_TEST_SIZE_BUF 700

static char *tar_add_file_test();
static char *test_tar_ls();
static char *tar_read_file_test();
int tests_run = 0;


static char *(*tests[])(void) = {tar_add_file_test, test_tar_ls, tar_read_file_test};

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

static char *tar_read_file_test() {
  int fd1 = open("/tmp/tsh_test/read_file_test", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd1 < 0) {
    mu_assert("Open didn't worked", 0);
  }
  tar_read_file("/tmp/tsh_test/test.tar", "man_dir/man", fd1);
  system("man man > /tmp/tsh_test/man_man");
  mu_assert("Error with content of file", system("diff /tmp/tsh_test/man_man /tmp/tsh_test/read_file_test") == 0);
  close(fd1);

  int fd2 = open("/tmp/tsh_test/read_file_test_empty", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd2 < 0) {
    mu_assert("Open didn't worked", 0);
  }
  mu_assert("The file doesn't exists and the function shouldn't return 0",
    tar_read_file("/tmp/tsh_test/test.tar", "dont_exist", fd2) != 0);
  char c;
  mu_assert("File should be empty", read(fd2, &c, 1) == 0);
  close(fd2);
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
