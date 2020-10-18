#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tar.h"
#include "path_lib.h"

#define CMD_NAME "cat"

int main(int argc, char *argv[]) {
  if (argc == 1) {
    execlp(CMD_NAME, CMD_NAME, NULL);
  }
  int ret = EXIT_SUCCESS;
  for (int i = 1; i < argc; i++) {
    char *in_tar = split_tar_abs_path(argv[i]);
    if (*in_tar == '\0') {
      int f = fork(), w;
      switch(f) {
        case -1:
          perror("fork");
          break;
        case 0: // son
          execlp(CMD_NAME, CMD_NAME, argv[i], NULL);
        default:
          wait(&w);
      }
    }
    else {
      tar_cp_file(argv[i], in_tar, STDOUT_FILENO);
    }

  }
  exit(ret);
}
