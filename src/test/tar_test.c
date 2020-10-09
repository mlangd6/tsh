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
static char *is_tar_test();
int tests_run = 0;

static char *(*tests[])(void) = {tar_add_file_test, is_tar_test};

static char *tar_add_file_test() {
  return 0;
}

static char *is_tar_test() {
  // test intégrité valide
  mu_assert("Error, is_tar(\"/tmp/tsh_test/test.tar\") != 1", is_tar("/tmp/tsh_test/test.tar") == 1);

  // test fichier vide
  system("touch /tmp/tsh_test/toto");
  mu_assert("Error, is_tar(\"/tmp/tsh_test/toto\") != 0", is_tar("/tmp/tsh_test/toto") == 0);

  // test corruption
  int fd = open("/tmp/tsh_test/test.tar", O_RDWR); 
  lseek(fd, 148, SEEK_SET);
  char bad_chksm[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
  write(fd, bad_chksm, sizeof(bad_chksm));
  mu_assert("Error, is_tar(\"/tmp/tsh_test/test.tar\") != 0", is_tar("/tmp/tsh_test/test.tar") == 0);  
  close(fd);
  
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
