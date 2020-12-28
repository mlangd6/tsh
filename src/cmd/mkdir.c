#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "tar.h"
#include "errors.h"
#include "command_handler.h"
#include "utils.h"
#include "path_lib.h"

#define CMD_NAME "mkdir"

int mkdir(char *tar_name, char *filename, char *options)
{
  int len_tar_name = strlen(tar_name);
  int len_filename = strlen(filename);
  char err[len_tar_name + len_filename + 30];
  sprintf(err, "cannot create directory \'%s/%s\'", tar_name, filename);
  if (is_empty_string(filename))
  {
    errno = EEXIST;
    error_cmd(CMD_NAME, err);
    return EXIT_FAILURE;
  }
  switch(type_of_file(tar_name, filename, true))
  {
    case REG:
    case DIR:
      errno = EEXIST;
      error_cmd(CMD_NAME, err);
      return EXIT_FAILURE;
    case NONE:
      if (errno != ENOENT)
      {
        if (errno == ENOTDIR)
          errno = EEXIST;
        error_cmd(CMD_NAME, err);
        return EXIT_FAILURE;
      }
      break;
  }
  char *dir = append_slash(filename);
  if(tar_add_file(tar_name, NULL, dir) != 0)
  {
    free(dir);
    error_cmd(CMD_NAME, err);
    return EXIT_FAILURE;
  }
  free(dir);
  return EXIT_SUCCESS;
}


int main(int argc, char *argv[]){
  unary_command cmd = {
    CMD_NAME,
    mkdir,
    false,
    false,
    ""
  };
  return handle_unary_command (cmd, argc, argv);
}
