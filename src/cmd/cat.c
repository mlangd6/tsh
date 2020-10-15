#include "tar.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define CMD_NAME "cat"

int main(int argc, char const *argv[]) {
  if (argc == 1) {
    execlp(CMD_NAME, CMD_NAME, NULL);
  }
  int ret = EXIT_SUCCESS;
  char *wd = getcwd(NULL, 0);
  char *pwd = getenv("PWD");
  for (int i = 1; i < argc; i++) {
    char *tar = get_tar_dir(argv[i]);
    if (tar[0] == '\0') {
      tar = pwd + strlen(wd) + 1;
      tar_read_file(tar, argv[i], STDOUT_FILENO);
    }

  }
  free(wd);
  exit(ret);
}
