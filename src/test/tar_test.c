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
#define TAR_TEST_SIZE 6
#define TAR_ADD_TEST_SIZE_BUF 700


static char *tar_add_file_test();
static char *test_tar_ls();
static char *is_tar_test();
static char *tar_cp_test();
static char *tar_rm_test();
static char *tar_mv_test();

int tests_run = 0;

static char *(*tests[])(void) = { tar_add_file_test, test_tar_ls, is_tar_test, tar_cp_test, tar_rm_test, tar_mv_test};

static char *stat_equals(struct stat *s1, struct stat *s2) {
  mu_assert("tar_add_file_test: error: st_mode", s1 -> st_mode == s2 -> st_mode);
  mu_assert("tar_add_file_test: error: st_uid", s1 -> st_uid == s2 -> st_uid);
  mu_assert("tar_add_file_test: error: st_gid", s1 -> st_gid == s2 -> st_gid);
  mu_assert("tar_add_file_test: error: st_size", s1 -> st_size == s2 -> st_size);
  return 0;
}

static char *tar_add_file_test() {
  char *tmp = getcwd(NULL, 0);
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
  close(fd2);
  char *s = stat_equals(&s1, &s2);
  if (s != 0) {
    return s;
  }
  mu_assert("tar_add_file_test: error: content of file", strncmp(buff1, buff2, TAR_ADD_TEST_SIZE_BUF) == 0);
  chdir(tmp);
  free(tmp);
  return 0;
}

static char *is_tar_test() {
  // test intégrité valide
  mu_assert("Error, is_tar(\"/tmp/tsh_test/test.tar\") != 1", is_tar("/tmp/tsh_test/test.tar") == 1);

  // test fichier vide
  system("touch /tmp/tsh_test/toto");
  mu_assert("Error, is_tar(\"/tmp/tsh_test/toto\") != 0", is_tar("/tmp/tsh_test/toto") == 0);

  // test corruption
  char bad_chksm[8];
  memset(bad_chksm, '\0', sizeof(bad_chksm));
  int fd = open("/tmp/tsh_test/test.tar", O_RDWR);
  lseek(fd, 148, SEEK_SET);
  write(fd, bad_chksm, sizeof(bad_chksm));
  mu_assert("Error, is_tar(\"/tmp/tsh_test/test.tar\") != 0", is_tar("/tmp/tsh_test/test.tar") == 0);
  close(fd);

  return 0;
}


static char *test_tar_ls(){
  int tmp;
  char *test[12] = {"dir1/", "dir1/subdir/", "dir1/subdir/subsubdir/", "dir1/subdir/subsubdir/hello", "dir1/tata", "man_dir/", "man_dir/man", "man_dir/open2", "man_dir/tar", "titi", "titi_link", "toto"};
  struct posix_header *a_tester = tar_ls("/tmp/tsh_test/test.tar");
  for(int i = 0; i < 12; i++) {
    tmp = 0;
    for(int j = 0; j < 12; j++)
      mu_assert("Error, this isn't the good ls", strcmp(test[i], a_tester[j].name) == 0 || tmp++ < 12 );
  }
  free(a_tester);
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

static char *tar_rm_test()
{
  mu_assert("Couldn't remove man_dir/open2", tar_rm_file("/tmp/tsh_test/test.tar", "man_dir/open2") == 0);
  mu_assert("Error tar_rm_file corrupted the tar", is_tar("/tmp/tsh_test/test.tar") == 1);

  mu_assert("tar_rm_file(\"/tmp/tsh_test/test.tar\", \"man_dir/open2\") != -1", tar_rm_file("/tmp/tsh_test/test.tar", "man_dir/open2") == -1);

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
