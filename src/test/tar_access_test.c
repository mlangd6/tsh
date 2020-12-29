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
#include "tar_access_test.h"



static char *is_tar_test();
static char *tar_access_test();

extern int tests_run;

static char *(*tests[])(void) = {
  is_tar_test,
  tar_access_test,
};

static char *all_tests() {
  for (int i = 0; i < TAR_TEST_SIZE; i++) {
    before();
    mu_run_test(tests[i]);
  }
  return 0;
}

int launch_tar_access_tests() {
  int prec_tests_run = tests_run;
  char *results = all_tests();
  if (results != 0) {
    printf(RED "%s\n" WHITE, results);
  }
  else {
    printf(GREEN "ALL TAR ACCESS TESTS PASSED\n" WHITE);
  }
  printf("tar access tests run: %d\n\n", tests_run - prec_tests_run);
  return (results == 0);
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
