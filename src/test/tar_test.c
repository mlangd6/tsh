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
#define TAR_TEST_SIZE 1
#define TAR_ADD_TEST_SIZE_BUF 700

static char *tar_add_file_test();
int tests_run = 0;

static char *(*tests[])(void) = {tar_add_file_test};

static char *tar_add_file_test() {
  chdir(TEST_DIR);
  char buff1[TAR_ADD_TEST_SIZE_BUF];
  memset(buff1, 'a', TAR_ADD_TEST_SIZE_BUF);

  int fd = open("tar_test", O_CREAT | O_WRONLY, 0600);
  write(fd, buff1, TAR_ADD_TEST_SIZE_BUF);
  close(fd);
  struct stat s1, s2;
  stat("tar_test", &s1);
  tar_add_file("test.tar", "tar_test");
  system("rm tar_test");
  system("tar -xf test.tar tar_test");

  stat("tar_test", &s2);
  int fd2 = open("tar_test", O_RDONLY);
  char buff2[TAR_ADD_TEST_SIZE_BUF];
  read(fd2, buff2, TAR_ADD_TEST_SIZE_BUF);
  mu_assert("tar_add_file_test: error, st_mode of file", s1.st_mode == s2.st_mode);
  mu_assert("tar_add_file_test: error, initial file content differents from after extarct file content", strcmp(buff1, buff2) == 0);
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
