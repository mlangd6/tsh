#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "minunit.h"
#include "tsh_test.h"
#include "tar.h"

#define TAR_TEST_SIZE 10
#define TAR_ADD_TEST_SIZE_BUF 700


static char *tar_add_file_test();
static char *test_tar_ls();
static char *is_tar_test();
static char *tar_cp_test();
static char *tar_rm_file_test();
static char *tar_rm_dir_test();
static char *tar_mv_test();
static char *tar_access_test();
static char *tar_append_file_test();
static char *tar_add_file_rec_test();

extern int tests_run;

static char *(*tests[])(void) = {
  tar_add_file_test,
  test_tar_ls,
  is_tar_test,
  tar_cp_test,
  tar_rm_file_test,
  tar_rm_dir_test,
  tar_mv_test,
  tar_access_test,
  tar_append_file_test,
  tar_add_file_rec_test
};

static char *stat_equals(struct stat *s1, struct stat *s2) {
  mu_assert("tar_add_file_test: error: st_mode", s1 -> st_mode == s2 -> st_mode);
  mu_assert("tar_add_file_test: error: st_uid", s1 -> st_uid == s2 -> st_uid);
  mu_assert("tar_add_file_test: error: st_gid", s1 -> st_gid == s2 -> st_gid);
  mu_assert("tar_add_file_test: error: st_size", s1 -> st_size == s2 -> st_size);
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
  system("tar -xf /tmp/tsh_test/test.tar tar_test");

  stat("tar_test", &s2);
  int fd2 = open("tar_test", O_RDONLY);
  char buff2[TAR_ADD_TEST_SIZE_BUF];
  read(fd2, buff2, TAR_ADD_TEST_SIZE_BUF);
  close(fd2);
  char *s = stat_equals(&s1, &s2);
  if (s != 0) {
    return s;
  }
  mu_assert("tar_add_file_test: 1 error: content of file", strncmp(buff1, buff2, TAR_ADD_TEST_SIZE_BUF) == 0);
  system("rm tar_test");


  system("touch /tmp/tsh_test/taitai");
  system("ln -s /tmp/tsh_test/taitai /tmp/tsh_test/taitai_link");
  //system("ls -l /tmp/tsh_test/");
  tar_add_file("/tmp/tsh_test/test.tar", "/tmp/tsh_test/taitai_link", "taitai_link");
  //system("tar tvf /tmp/tsh_test/test.tar");
  int n_b;
  struct posix_header *hd = tar_ls("/tmp/tsh_test/test.tar", &n_b);
  int a = 0;
  for(int i = 0; i < n_b; i++)
      if(strcmp(hd[i].linkname, "/tmp/tsh_test/taitai") == 0)a++;
  mu_assert("tar_add_file_test: 2 error: content of file", a == 1);
  /*char buff_1[TAR_ADD_TEST_SIZE_BUF];
  memset(buff_1, 'a', TAR_ADD_TEST_SIZE_BUF);

  int fd_ = open("/tmp/tsh_test/taitai_link", O_CREAT | O_WRONLY, 0600);
  write(fd_, buff_1, TAR_ADD_TEST_SIZE_BUF);
  close(fd_);
  struct stat s1b, s2b;
  stat("/tmp/tsh_test/taitai_link", &s1b);*/
  system("rm /tmp/tsh_test/taitai_link");
  /*system("tar -xf /tmp/tsh_test/test.tar taitai_link");
  system("ls -l /tmp/tsh_test");
  stat("/tmp/tsh_test/taitai_link", &s2b);
  int fd_2 = open("taitai_link", O_RDONLY);
  char buff_2[TAR_ADD_TEST_SIZE_BUF];
  read(fd_2, buff_2, TAR_ADD_TEST_SIZE_BUF);
  close(fd_2);
  char *sb = stat_equals(&s1b, &s2b);
  if (sb != 0) {
    return sb;
  }
  mu_assert("tar_add_file_test: 2 error: content of file", strncmp(buff_1, buff_2, TAR_ADD_TEST_SIZE_BUF) == 0);
  system("rm /tmp/tsh_test/taitai /tmp/tsh_test/taitai_link");*/


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

static char *tar_add_file_rec_test() {
  int nb = 0;
  struct posix_header *a_tester = tar_ls("/tmp/tsh_test/test.tar", &nb);
  for(int i = 0; i < nb; i++){
    mu_assert("tar_add_file_rec_test: error: \"./src/cmd/ls.c\" is already in the tar", strcmp("dir1/src/cmd/ls.c", a_tester[i].name) != 0);
  }
  tar_add_file_rec("/tmp/tsh_test/test.tar", ".", "dir1/tsh/", 0);
  int nb2 = 0;
  struct posix_header *a_tester2 = tar_ls("/tmp/tsh_test/test.tar", &nb2);
  int tmp[4] = {nb2, nb2, nb2, nb2};
  for(int i = 0; i < nb2; i++){
    if(strcmp("dir1/tsh/", a_tester2[i].name) == 0)tmp[0] = i;
    if(strcmp("dir1/tsh/src/cmd/ls.c", a_tester2[i].name) == 0)tmp[1] = i;
    if(strcmp("dir1/tsh/bin/", a_tester2[i].name) == 0)tmp[2] = i;
    if(strcmp("dir1/tsh/target/cmd/ls.o", a_tester2[i].name) == 0)tmp[3] = i;
  }
  for(int i = 0; i < 4; i++){
    mu_assert("tar_add_file_test: error: \"./\" isn't add in the tar", tmp[i] < nb2 );
  }
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
  int tmp, size = 0;
  char *test[] =
  {"dir1/", "dir1/subdir/", "dir1/subdir/subsubdir/",
  "dir1/subdir/subsubdir/hello", "dir1/tata", "man_dir/", "man_dir/man",
  "man_dir/open2", "man_dir/tar", "titi",
  "titi_link", "toto", "dir2/fic1", "dir2/fic2", "access/",
  "access/x", "access/no_x_dir/", "access/no_x_dir/a"};
  struct posix_header *a_tester = tar_ls("/tmp/tsh_test/test.tar", &size);
  for(int i = 0; i < size; i++) {
    tmp = 0;
    for(int j = 0; j < size; j++)
      mu_assert("Error, this isn't the good ls", strcmp(test[i], a_tester[j].name) == 0 || tmp++ < size );
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

static char *tar_rm_file_test()
{
  mu_assert("Couldn't remove man_dir/open2", tar_rm("/tmp/tsh_test/test.tar", "man_dir/open2") == 0);
  mu_assert("Error tar_rm corrupted the tar", is_tar("/tmp/tsh_test/test.tar") == 1);

  mu_assert("tar_rm(\"/tmp/tsh_test/test.tar\", \"man_dir/open2\") != -2", tar_rm("/tmp/tsh_test/test.tar", "man_dir/open2") == -2);

  return 0;
}

static char *tar_rm_dir_test()
{
  mu_assert("Couldn't remove dir1/", tar_rm("/tmp/tsh_test/test.tar","dir1/") == 0);
  mu_assert("Error tar_rm corrupted the tar", is_tar("/tmp/tsh_test/test.tar") == 1);

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

static char *tar_access_test()
{
  int is_root = !getuid();
  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"titi_link\", F_OK) != 1", tar_access("/tmp/tsh_test/test.tar", "titi_link", F_OK) == 1);
  int not_arg = (~F_OK & ~R_OK & ~W_OK & ~X_OK);
  errno = 0;
  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"titi_link\", !0) != -1", tar_access("/tmp/tsh_test/test.tar", "titi_link", not_arg) == -1);
  mu_assert("errno != ENOVAL after tar_access(\"/tmp/tsh_test/test.tar\", \"titi_link\", !0)", errno == EINVAL);

  errno = 0;
  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"man_dir/titi_link\", F_OK) != -1", tar_access("/tmp/tsh_test/test.tar", "man_dir/titi_link", F_OK) == -1);
  mu_assert("errno != ENOENT after tar_access(\"/tmp/tsh_test/test.tar\", \"man_dir/titi_link\", F_OK)", errno == ENOENT);

  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"dir1/\", F_OK) != 1", tar_access("/tmp/tsh_test/test.tar", "dir1/", F_OK) == 1);

  errno = 0;
  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"dirNot/\", F_OK) != -1", tar_access("/tmp/tsh_test/test.tar", "dirNot/", F_OK) == -1);
  mu_assert("errno != ENOENT after tar_access(\"/tmp/tsh_test/test.tar\", \"dirNot/\", F_OK)", errno == ENOENT);
  errno = 0;
  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"dirNot/\", R_OK|W_OK|X_OK) != -1", tar_access("/tmp/tsh_test/test.tar", "dirNot/", R_OK|W_OK|X_OK) == -1);
  mu_assert("errno != ENOENT after tar_access(\"/tmp/tsh_test/test.tar\", \"dirNot/\", R_OK|W_OK|X_OK)", errno == ENOENT);

  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"dir2/\", F_OK) != 2", tar_access("/tmp/tsh_test/test.tar", "dir2/", F_OK) == 2);
  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"dir2/\", R_OK | W_OK | X_OK) != 2", tar_access("/tmp/tsh_test/test.tar", "dir2/", R_OK | W_OK | X_OK) == 2);

  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"dir2/fic1\", F_OK) != 1", tar_access("/tmp/tsh_test/test.tar", "dir2/fic1", F_OK) == 1);

  int test_value = is_root ? 1 : -1;
  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"access/no_x_dir/a\"), F_OK) error", tar_access("/tmp/tsh_test/test.tar", "access/no_x_dir/a", F_OK) == test_value);
  if (! is_root)
    mu_assert("errno != EACCES after tar_access(\"/tmp/tsh_test/test.tar\", \"access/no_x_dir/a\"), F_OK)", errno == EACCES);
  errno = 0;
  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"access/no_x_dir/a\"), R_OK) error", tar_access("/tmp/tsh_test/test.tar", "access/no_x_dir/a", R_OK) == test_value);
  if (! is_root)
    mu_assert("errno != EACCES after tar_access(\"/tmp/tsh_test/test.tar\", \"access/no_x_dir/a\"), R_OK)", errno == EACCES);

  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"access/x\"), X_OK) != 1", tar_access("/tmp/tsh_test/test.tar", "access/x", X_OK) == 1);
  errno = 0;
  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"access/x\"), R_OK) error", tar_access("/tmp/tsh_test/test.tar", "access/x", R_OK) == test_value);
  if (! is_root)
    mu_assert("errno != EACCES after tar_access(\"/tmp/tsh_test/test.tar\", \"access/x\"), R_OK)", errno == EACCES);;
  errno = 0;
  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"access/x\"), W_OK) error", tar_access("/tmp/tsh_test/test.tar", "access/x", W_OK) == test_value);
  if (! is_root)
    mu_assert("errno != EACCES after tar_access(\"/tmp/tsh_test/test.tar\", \"access/x\"), X_OK)", errno == EACCES);;

  return 0;
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

static char *all_tests() {
  for (int i = 0; i < TAR_TEST_SIZE; i++) {
    before();
    mu_run_test(tests[i]);
  }
  return 0;
}

int launch_tar_tests() {
  int prec_tests_run = tests_run;
  char *results = all_tests();
  if (results != 0) {
    printf("%s\n", results);
  }
  else {
    printf("ALL TAR TESTS PASSED\n");
  }
  printf("tar tests run: %d\n\n", tests_run - prec_tests_run);
  return (results == 0);
}
