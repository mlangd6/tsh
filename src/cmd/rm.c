#include "tar.h"
#include "errors.h"
#include "command_handler.h"
#include "utils.h"
#include "path_lib.h"
#include "remove.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>



int main(int argc, char *argv[])
{
  unary_command cmd = {
    CMD_NAME_,
    rm,
    false,
    false,
    SUPPORT_OPT_
  };
  return handle_unary_command (cmd, argc, argv);
}
