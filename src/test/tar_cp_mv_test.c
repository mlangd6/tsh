#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "tsh_test.h"
#include "minunit.h"
#include "tar.h"
#include "tar_cp_mv_test.h"

extern int tests_run;

static char *tar_cp_test();
static char *tar_mv_test();
static char *tar_extract_dir_man_dir_test();
static char *all_tests();

static char *(*tests[])(void) = {
  tar_cp_test,
  tar_mv_test,
  tar_extract_dir_man_dir_test
};

int launch_tar_cp_mv_tests() {
  int prec_tests_run = tests_run;
  char *results = all_tests();
  if (results != 0) {
    printf(RED "%s\n" WHITE, results);
  }
  else {
    printf(GREEN "ALL TAR MV/CP TESTS PASSED\n" WHITE);
  }
  printf("tar mv/cp tests run: %d\n\n", tests_run - prec_tests_run);
  return (results == 0);
}


static char *all_tests()
{
  for (int i = 0; i < TAR_CP_MV_TEST_SIZE; i++)
    {
      before();
      mu_run_test(tests[i]);
    }
  return 0;
}

static char *tar_cp_test() {
  int fd1 = open("/tmp/tsh_test/cp_test", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd1 < 0) {
    mu_assert("Open didn't worked", 0);
  }
  tar_cp_file("/tmp/tsh_test/test.tar", "man_dir/man", fd1);
  system("man man > /tmp/tsh_test/man_man");
  mu_assert("Error with content of file", system("diff /tmp/tsh_test/man_man /tmp/tsh_test/cp_test") == 0);
  close(fd1);

  int fd2 = open("/tmp/tsh_test/cp_test_empty", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd2 < 0) {
    mu_assert("Open didn't worked", 0);
  }
  mu_assert("The file doesn't exists and the function shouldn't return 0",
	    tar_cp_file("/tmp/tsh_test/test.tar", "dont_exist", fd2) != 0);
  char c;
  mu_assert("File should be empty", read(fd2, &c, 1) == 0);
  close(fd2);
  return 0;
}

static char *tar_extract_dir_man_dir_test()
{
  int r = tar_extract_dir("/tmp/tsh_test/test.tar", "man_dir/", "/tmp/tsh_test");
  mu_assert("Error during the extraction", r == 0);

  system("man man > /tmp/tsh_test/man_dir/man_diff;"		\
	 "man 2 open > /tmp/tsh_test/man_dir/open2_diff;"	\
	 "man tar > /tmp/tsh_test/man_dir/tar_diff");
  
  mu_assert("system(\"diff /tmp/tsh_test/man_dir/man_diff /tmp/tsh_test/man_dir/man\") == 0)", system("diff /tmp/tsh_test/man_dir/man_diff /tmp/tsh_test/man_dir/man") == 0);
  mu_assert("system(\"diff /tmp/tsh_test/man_dir/open2_diff /tmp/tsh_test/man_dir/open2\") == 0)", system("diff /tmp/tsh_test/man_dir/open2_diff /tmp/tsh_test/man_dir/open2") == 0);
  mu_assert("system(\"diff /tmp/tsh_test/man_dir/tar_diff /tmp/tsh_test/man_dir/tar\") == 0)", system("diff /tmp/tsh_test/man_dir/tar_diff /tmp/tsh_test/man_dir/tar") == 0);
  
  system("rm -r /tmp/tsh_test/man_dir");  
  
  return 0;
}



static char *tar_mv_test()
{
  int fd = open("/tmp/tsh_test/mv_test", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  mu_assert("Open didn't work", fd > 0);

  mu_assert("Couldn't mv \"/tmp/tsh_test/test.tar/man_dir/man\"", tar_mv_file("/tmp/tsh_test/test.tar", "man_dir/man", fd) == 0);

  system("man man > /tmp/tsh_test/man_man");
  mu_assert("Error with content of file", system("diff /tmp/tsh_test/man_man /tmp/tsh_test/mv_test") == 0);

  mu_assert("tar_mv_file(\"/tmp/tsh_test/test.tar\", \"man_dir/man\", fd) != -1", tar_mv_file("/tmp/tsh_test/test.tar", "man_dir/man", fd) == -1);

  close(fd);

  return 0;
}
