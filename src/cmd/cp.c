#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "command_handler.h"
#include "copy.h"

#define CMD_NAME "cp"


int main (int argc, char *argv[])
{
  set_cmd_name(CMD_NAME);
  binary_command cmd =
    {
      CMD_NAME,
      cp_tar_to_tar,
      cp_ext_to_tar,
      cp_tar_to_ext,
      SUPPORT_OPT
    };

  return handle_binary_command (cmd, argc, argv);
}
