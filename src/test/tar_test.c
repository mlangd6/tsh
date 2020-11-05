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
  char *tmp = getcwd(NULL, 0);
  chdir(TEST_DIR);
  char buff1[TAR_ADD_TEST_SIZE_BUF];
  memset(buff1, 'a', TAR_ADD_TEST_SIZE_BUF);

  int fd = open("tar_test", O_CREAT | O_WRONLY, 0600);
  write(fd, buff1, TAR_ADD_TEST_SIZE_BUF);
  close(fd);
  struct stat s1, s2;
  stat("tar_test", &s1);
  tar_add_file("test.tar", "tar_test", "tar_test");
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
  tar_add_file("test.tar", NULL, "toto_test");
  tar_add_file("test.tar", NULL, "dir1/dir_test/");
  struct posix_header *a_tester = tar_ls("/tmp/tsh_test/test.tar");
  mu_assert("tar_add_file_test: error: \"toto_test\" isn't add in the tar", strcmp("toto_test", a_tester[15].name) == 0);
  mu_assert("tar_add_file_test: error: \"dir1/titi_test/\" isn't add in the tar", strcmp("dir1/dir_test/", a_tester[16].name) == 0);
  chdir(tmp);
  free(tmp);
  return 0;
}

static char *tar_add_file_rec_test() {
  struct posix_header *a_tester = tar_ls("/tmp/tsh_test/test.tar");
  int fd = open("/tmp/tsh_test/test.tar", O_RDONLY);
  int nb = nb_files_in_tar(fd);
  close(fd);
  for(int i = 0; i < nb; i++){
    mu_assert("tar_add_file_rec_test: error: \"./src/cmd/ls.c\" is already in the tar", strcmp("dir1/src/cmd/ls.c", a_tester[i].name) != 0);
  }
  tar_add_file_rec("/tmp/tsh_test/test.tar", ".", "dir1/tsh/", 0);
  struct posix_header *a_tester2 = tar_ls("/tmp/tsh_test/test.tar");
  int fd2 = open("/tmp/tsh_test/test.tar", O_RDONLY);
  int nb2 = nb_files_in_tar(fd2);
  close(fd2);
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
  int tmp;
  char *test[14] = {"dir1/", "dir1/subdir/", "dir1/subdir/subsubdir/", "dir1/subdir/subsubdir/hello", "dir1/tata", "man_dir/", "man_dir/man",
                    "man_dir/open2", "man_dir/tar", "titi", "titi_link", "toto", "dir2/fic1", "dir2/fic2"};
  struct posix_header *a_tester = tar_ls("/tmp/tsh_test/test.tar");
  for(int i = 0; i < 14; i++) {
    tmp = 0;
    for(int j = 0; j < 14; j++)
      mu_assert("Error, this isn't the good ls", strcmp(test[i], a_tester[j].name) == 0 || tmp++ < 14 );
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
  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"titi_link\", F_OK) != 1", tar_access("/tmp/tsh_test/test.tar", "titi_link", F_OK) == 1);

  errno = 0;
  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"titi_link\", !F_OK) != -1", tar_access("/tmp/tsh_test/test.tar", "titi_link", !F_OK) == -1);
  mu_assert("errno != ENOVAL", errno == EINVAL);

  errno = 0;
  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"man_dir/titi_link\", F_OK) != -1", tar_access("/tmp/tsh_test/test.tar", "man_dir/titi_link", F_OK) == -1);
  mu_assert("errno != ENOENT", errno == ENOENT);

  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"dir1/\", F_OK) != 1", tar_access("/tmp/tsh_test/test.tar", "dir1/", F_OK) == 1);

  errno = 0;
  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"dirNot/\", F_OK) != -1", tar_access("/tmp/tsh_test/test.tar", "dirNot/", F_OK) == -1);
  mu_assert("errno != ENOENT", errno == ENOENT);

  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"dir2/\", F_OK) != 2", tar_access("/tmp/tsh_test/test.tar", "dir2/", F_OK) == 2);

  mu_assert("tar_access(\"/tmp/tsh_test/test.tar\", \"dir2/fic1\", F_OK) != 1", tar_access("/tmp/tsh_test/test.tar", "dir2/fic1", F_OK) == 1);
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
  system("tar -xf /tmp/tsh_test/test.tar -C /tmp/tsh_test/");

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
