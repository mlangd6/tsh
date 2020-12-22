#include <stdbool.h>
#include <stdio.h>

#include "command_handler.h"
#include "copy.h"


int main (int argc, char *argv[])
{
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
