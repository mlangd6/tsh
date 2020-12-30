#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "command_handler.h"
#include "errors.h"
#include "path_lib.h"
#include "tar.h"
#include "utils.h"

#define CMD_NAME "mkdir"

static void mkdir_error(int new_errno, char *tar_name, char *filename)
{
  error(new_errno, "%s: cannot create directory \'%s/%s\'", CMD_NAME, tar_name, filename);
}

int mkdir(char *tar_name, char *filename, char *options)
{
  if (is_empty_string(filename))
  {
    mkdir_error(EEXIST, tar_name, filename);
    return EXIT_FAILURE;
  }
  switch(type_of_file(tar_name, filename, true))
  {
    case REG:
    case DIR:
      errno = EEXIST;
      mkdir_error(EEXIST, tar_name, filename);
      return EXIT_FAILURE;
    case NONE:
      if (errno != ENOENT)
      {
        if (errno == ENOTDIR)
          errno = EEXIST;
        mkdir_error(errno, tar_name, filename);
        return EXIT_FAILURE;
      }
      break;
  }
  char *dir = append_slash(filename);
  if(tar_add_file(tar_name, NULL, dir) != 0)
  {
    free(dir);
    mkdir_error(errno, tar_name, filename);
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
