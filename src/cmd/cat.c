#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "tar.h"
#include "path_lib.h"
#include "errors.h"

#define CMD_NAME "cat"

int main(int argc, char *argv[]) {
  if (argc == 1) {
    execlp(CMD_NAME, CMD_NAME, NULL);
  }
  if (argv[1][0] == '-') {
    execvp(argv[0], argv);
    return EXIT_FAILURE;
  }
  for (int i = 1; i < argc; i++) {
    char *in_tar = split_tar_abs_path(argv[i]);
    if (*in_tar == '\0') {
      int f = fork(), w;
      switch(f) {
        case -1:
          error_cmd(CMD_NAME, "fork");
          return EXIT_FAILURE;
          break;
        case 0: // son
          execlp(CMD_NAME, CMD_NAME, argv[i], NULL);
        default:
          wait(&w);
      }
    }
    else {
      if (tar_cp_file(argv[i], in_tar, STDOUT_FILENO) != 0) {
        in_tar[-1] = '/';
        error_cmd(CMD_NAME, argv[i]);
        return EXIT_FAILURE;
      }
    }
  }
  return EXIT_SUCCESS;
}
