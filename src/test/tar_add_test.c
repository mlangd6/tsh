#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "tsh_test.h"
#include "minunit.h"
#include "tar.h"
#include "tar_add_test.h"

extern int tests_run;

static char *tar_add_file_rec_test();
static char *add_tar_file_in_tar_test();
static char *tar_add_file_test();
static char *tar_append_file_test();
static char *tar_add_file_no_source_test();
static char *tar_add_file_link_test();
static char *move_file_to_end_of_tar_test();

static char *all_tests();

static char *(*tests[])(void) = {
  tar_add_file_test,
  tar_add_file_link_test,
  tar_add_file_no_source_test,
  tar_add_file_rec_test,
  add_tar_file_in_tar_test,
  tar_append_file_test,
  move_file_to_end_of_tar_test
};


int launch_tar_add_tests()
{
  int prec_tests_run = tests_run;
  char *results = all_tests();
  if (results != 0) {
    printf(RED "%s\n" WHITE, results);
  }
  else {
    printf(GREEN "ALL TAR ADD FILE TESTS PASSED\n" WHITE);
  }
  printf("tar add file tests run: %d\n\n", tests_run - prec_tests_run);
  return (results == 0);
}

static char *all_tests()
{
  for (int i = 0; i < TAR_ADD_FILE_TEST_SIZE; i++)
  {
    before();
    mu_run_test(tests[i]);
  }
  return 0;
}

static char *stat_equals(struct stat *s1, struct stat *s2) {
  mu_assert("tar_add_file_test: error: st_mode", s1 -> st_mode == s2 -> st_mode);
  mu_assert("tar_add_file_test: error: st_uid", s1 -> st_uid == s2 -> st_uid);
  mu_assert("tar_add_file_test: error: st_gid", s1 -> st_gid == s2 -> st_gid);
  mu_assert("tar_add_file_test: error: st_size", s1 -> st_size == s2 -> st_size);
  return 0;
}

static char *tar_add_file_link_test()
{
  char buff_1[TAR_ADD_TEST_SIZE_BUF];
  memset(buff_1, 'a', TAR_ADD_TEST_SIZE_BUF);
  int fd = open("/tmp/tsh_test/taitai", O_CREAT | O_WRONLY, 0600);
  write(fd, buff_1, TAR_ADD_TEST_SIZE_BUF);
  close(fd);
  system("ln -s /tmp/tsh_test/taitai /tmp/tsh_test/taitai_link");
  tar_add_file("/tmp/tsh_test/test.tar", "/tmp/tsh_test/taitai_link", "taitai_link");
  struct stat s1b, s2b;
  lstat("/tmp/tsh_test/taitai_link", &s1b);
  system("rm /tmp/tsh_test/taitai_link");
  system("tar -C /tmp/tsh_test -xf /tmp/tsh_test/test.tar taitai_link");
  lstat("/tmp/tsh_test/taitai_link", &s2b);
  fd = open("/tmp/tsh_test/taitai_link", O_RDONLY);
  char buff_2[TAR_ADD_TEST_SIZE_BUF];
  memset(buff_2, '\0', TAR_ADD_TEST_SIZE_BUF);
  read(fd, buff_2, TAR_ADD_TEST_SIZE_BUF);
  close(fd);
  char *sb = stat_equals(&s1b, &s2b);
  if (sb != 0) {
    return sb;
  }
  mu_assert("tar_add_file_test: 2 error: content of file", strncmp(buff_1, buff_2, TAR_ADD_TEST_SIZE_BUF) == 0);
  return 0;
}

static char *tar_add_file_no_source_test()
{
  tar_add_file("/tmp/tsh_test/test.tar", NULL, "toto_test");
  tar_add_file("/tmp/tsh_test/test.tar", NULL, "dir1/dir_test/");
  int nb;
  struct posix_header *a_tester = tar_ls("/tmp/tsh_test/test.tar", &nb);
  int test[] = {0, 0};
  for (int i = 0; i < nb; i++)
  {
    if (strcmp(a_tester[i].name, "toto_test") == 0) test[0]++;
    if (strcmp(a_tester[i].name, "dir1/dir_test/") == 0) test[1]++;
  }
  mu_assert("tar_add_file_test: error: \"toto_test\" isn't add in the tar", test[0] == 1);
  mu_assert("tar_add_file_test: error: \"dir1/dir_test/\" isn't add in the tar", test[1] == 1);
  free(a_tester);
  return 0;
}

static char *tar_add_file_test() {
  char buff1[TAR_ADD_TEST_SIZE_BUF];
  memset(buff1, 'a', TAR_ADD_TEST_SIZE_BUF);

  int fd = open("/tmp/tsh_test/tar_test", O_CREAT | O_WRONLY, 0600);
  write(fd, buff1, TAR_ADD_TEST_SIZE_BUF);
  close(fd);
  struct stat s1, s2;
  stat("/tmp/tsh_test/tar_test", &s1);
  tar_add_file("/tmp/tsh_test/test.tar", "/tmp/tsh_test/tar_test", "tar_test");
  system("rm /tmp/tsh_test/tar_test");
  system("tar -C /tmp/tsh_test -xf /tmp/tsh_test/test.tar tar_test");
  stat("/tmp/tsh_test/tar_test", &s2);
  int fd2 = open("/tmp/tsh_test/tar_test", O_RDONLY);
  char buff2[TAR_ADD_TEST_SIZE_BUF];
  memset(buff2, '\0', TAR_ADD_TEST_SIZE_BUF);
  read(fd2, buff2, TAR_ADD_TEST_SIZE_BUF);
  close(fd2);
  char *s = stat_equals(&s1, &s2);
  if (s != 0) {
    return s;
  }
  mu_assert("tar_add_file_test: 1 error: content of file", strncmp(buff1, buff2, TAR_ADD_TEST_SIZE_BUF) == 0);
  system("rm /tmp/tsh_test/tar_test");
  return 0;
}

static char *tar_add_file_rec_test() {
  system("mkdir -p /tmp/tsh_test/rec/sub_rec1/../sub_rec2");
  system("touch /tmp/tsh_test/rec/rec_f /tmp/tsh_test/rec/sub_rec1/sub_rec_f");
  tar_add_file_rec("/tmp/tsh_test/test.tar", "/tmp/tsh_test/rec", "dir1/rec_root/", 0);
  int nb;
  struct posix_header *a_tester = tar_ls("/tmp/tsh_test/test.tar", &nb);
  int tmp[] = {0, 0, 0, 0, 0};
  for(int i = 0; i < nb; i++){
    if(strcmp("dir1/rec_root/", a_tester[i].name) == 0)tmp[0]++;
    if(strcmp("dir1/rec_root/sub_rec1/", a_tester[i].name) == 0)tmp[1]++;
    if(strcmp("dir1/rec_root/sub_rec2/", a_tester[i].name) == 0)tmp[2]++;
    if(strcmp("dir1/rec_root/sub_rec1/sub_rec_f", a_tester[i].name) == 0)tmp[3]++;
    if(strcmp("dir1/rec_root/rec_f", a_tester[i].name) == 0)tmp[4]++;
  }
  for(int i = 0; i < 5; i++){
    mu_assert("tar_add_file_rec_test: error: \"./\" isn't add in the tar", tmp[i] == 1 );
  }
  return 0;
}

static char *add_tar_file_in_tar_test() {
  add_tar_file_in_tar_rec("/tmp/tsh_test/test.tar", "/tmp/tsh_test/bis_test.tar", "man_dir/", "man_dir_bis/man_dir/");
  int nb = 0;
  struct posix_header *a_tester = tar_ls("/tmp/tsh_test/bis_test.tar", &nb);
  int tmp[4] = {nb, nb, nb, nb};
  for(int i = 0; i < nb; i++){
    if(strcmp("man_dir_bis/man_dir/", a_tester[i].name) == 0)tmp[0] = i;
    if(strcmp("man_dir_bis/man_dir/tar", a_tester[i].name) == 0)tmp[1] = i;
    if(strcmp("man_dir_bis/man_dir/open2", a_tester[i].name) == 0)tmp[2] = i;
    if(strcmp("man_dir_bis/man_dir/man", a_tester[i].name) == 0)tmp[3] = i;
  }
  free(a_tester);
  for(int i = 0; i < 4; i++){
    mu_assert("tar_add_tar_file_test: error: 1, isn't add in the tar", tmp[i] < nb );
  }

  add_tar_file_in_tar_rec("/tmp/tsh_test/bis_test.tar", "/tmp/tsh_test/test.tar", "man_dir_bis/", "man_dir/man_dir_bis/");
  int nb2 = 0;
  struct posix_header *a_tester2 = tar_ls("/tmp/tsh_test/test.tar", &nb2);
  int tmp2[8] = {nb2, nb2, nb2, nb2, nb2, nb2, nb2, nb2};
  for(int i = 0; i < nb2; i++){
    if(strcmp("man_dir/man_dir_bis/", a_tester2[i].name) == 0)tmp2[0] = i;
    if(strcmp("man_dir/man_dir_bis/tar_bis", a_tester2[i].name) == 0)tmp2[1] = i;
    if(strcmp("man_dir/man_dir_bis/open2_bis", a_tester2[i].name) == 0)tmp2[2] = i;
    if(strcmp("man_dir/man_dir_bis/man_bis", a_tester2[i].name) == 0)tmp2[3] = i;
    if(strcmp("man_dir/man_dir_bis/man_dir/", a_tester2[i].name) == 0)tmp2[4] = i;
    if(strcmp("man_dir/man_dir_bis/man_dir/man", a_tester2[i].name) == 0)tmp2[5] = i;
    if(strcmp("man_dir/man_dir_bis/man_dir/open2", a_tester2[i].name) == 0)tmp2[6] = i;
    if(strcmp("man_dir/man_dir_bis/man_dir/tar", a_tester2[i].name) == 0)tmp2[7] = i;
  }
  free(a_tester2);
  for(int i = 0; i < 8; i++){
    mu_assert("tar_add_tar_file_test: error: 2, isn't add in the tar", tmp2[i] < nb2 );
  }
  return NULL;
}

static char *tar_append_file_test() {
  system("echo TEST> /tmp/tsh_test/append");
  system("truncate -s 50 /tmp/tsh_test/titi_append");
  system("echo TEST>>/tmp/tsh_test/titi_append");
  system("echo \"Hello World!\nTEST\"> /tmp/tsh_test/hello_append_test");
  int fd = open("/tmp/tsh_test/append", O_RDONLY);
  if (fd < 0) {
    mu_assert("open failed", 0);
  }
  tar_append_file("/tmp/tsh_test/test.tar", "titi", fd);
  lseek(fd, 0, SEEK_SET);
  tar_append_file("/tmp/tsh_test/test.tar", "dir1/subdir/subsubdir/hello", fd);
  mu_assert("append tar corrupted the tar", is_tar("/tmp/tsh_test/test.tar") == 1);
  system("tar -xf /tmp/tsh_test/test.tar -C /tmp/tsh_test/ titi dir1/subdir/subsubdir/hello");

  // Error msg will be made by cmp command if needed
  mu_assert("", system("cmp /tmp/tsh_test/titi_append /tmp/tsh_test/titi") == 0);
  mu_assert("", system("cmp /tmp/tsh_test/hello_append_test /tmp/tsh_test/dir1/subdir/subsubdir/hello") == 0);
  return 0;
}

static char *move_file_to_end_of_tar_test()
{
  move_file_to_end_of_tar("/tmp/tsh_test/test.tar", "titi");
  mu_assert("move_file_to_end_of_tar corrupted the tar", is_tar("/tmp/tsh_test/test.tar") == 1);
  mu_assert("titi is not at the end of the tar after launching move_file_to_end_of_tar function", system("2> /dev/null tar tvf /tmp/tsh_test/test.tar | tail -n 1 | grep titi > /dev/null") == 0);
  move_file_to_end_of_tar("/tmp/tsh_test/test.tar", "man_dir/man");
  mu_assert("move_file_to_end_of_tar corrupted the tar", is_tar("/tmp/tsh_test/test.tar") == 1);
  mu_assert("titi is still at the end of tar after launching move_file_to_end_of_tar on man_dir/man function", system("2> /dev/null tar tvf /tmp/tsh_test/test.tar | tail -n 1 | grep titi > /dev/null") != 0);
  mu_assert("man_dir/man is not at the end of the tar after launching move_file_to_end_of_tar function", system("2> /dev/null tar tvf /tmp/tsh_test/test.tar | tail -n 1 | grep man_dir/man > /dev/null") == 0);

  return 0;
}
