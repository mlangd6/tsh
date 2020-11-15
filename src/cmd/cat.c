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
#include "command_handler.h"

#define CMD_NAME "cat"

int cat(char *tar_name, char *filename, char *options)
{
  if (tar_cp_file(tar_name, filename, STDOUT_FILENO) != 0)
  {
    tar_name[strlen(tar_name)] = '/';
    error_cmd(CMD_NAME, tar_name);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
  command cmd = {
    CMD_NAME,
    cat,
    0,
    0,
    ""
  };
  return handle(cmd, argc, argv);
}
