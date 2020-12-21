#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "command_handler.h"
#include "errors.h"
#include "tar.h"

#define CMD_NAME "cat"

int cat (char *tar_name, char *filename, char *options)
{
  if (tar_cp_file(tar_name, filename, STDOUT_FILENO) != 0)
    {
      tar_name[strlen(tar_name)] = '/';
      error_cmd(CMD_NAME, tar_name);
      return EXIT_FAILURE;
    }
  
  return EXIT_SUCCESS;
}

int main (int argc, char *argv[])
{
  unary_command cmd =
    {
      CMD_NAME,
      cat,
      false,
      false,
      ""
    };
  
  return handle_unary_command (cmd, argc, argv);
}
