#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "command_handler.h"
#include "copy.h"

static void set_cmd_name()
{
  cmd_name[0] = '\0';
  strcpy(cmd_name, "cp");
}

int main (int argc, char *argv[])
{
  set_cmd_name();
  binary_command cmd =
    {
      cmd_name,
      cp_tar_to_tar,
      cp_ext_to_tar,
      cp_tar_to_ext,
      SUPPORT_OPT
    };

  return handle_binary_command (cmd, argc, argv);
}
