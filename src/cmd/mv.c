#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include "command_handler.h"
#include "copy.h"
#include "utils.h"
#include "remove.h"
#include "errors.h"

#define CMD_NAME__ "mv"
#define SUPPORT_OPT__ ""

static int do_rm(char *src_tar, char *src_file)
{
  unary_command cmd = {
    "rm",
    rm,
    false,
    false,
    "r"
  };
  char tmp[PATH_MAX];
  if(!is_empty_string(src_file))sprintf(tmp, "%s/%s", src_tar, src_file);
  else strcpy(tmp, src_tar);
  char *argv[3] = {"rm", "-r", tmp};
  if(handle_unary_command(cmd, 3, argv) < 0)
  {
    return -1;
  }
  return 0;
}

int mv_tar_to_tar (char *src_tar, char *src_file, char *dest_tar, char *dest_file, char *opt)
{
  if(cp_tar_to_tar(src_tar, src_file, dest_tar, dest_file, "r") < 0)
    return -1;

  if(do_rm(src_tar, src_file) < 0)
    return -1;

  return 0;
}

int mv_ext_to_tar (char *src_file, char *dest_tar, char *dest_file, char *opt)
{
  if(cp_ext_to_tar(src_file, dest_tar, dest_file, "r") < 0)
    return -1;

  int i = 0;
  switch(fork())
  {
    case -1:
      i = -1;
      break;
    case 0:
      execlp("rm", "rm", "-r", src_file, NULL);
      break;
    default:
      wait(NULL);
  }
  if(i == -1)
  {
    error_cmd(CMD_NAME__, src_file);
  }
  return 0;
}

int mv_tar_to_ext(char *src_tar, char *src_file, char *dest_file, char *opt)
{
  if(cp_tar_to_ext(src_tar, src_file, dest_file, "r") < 0)
    return -1;

  if(do_rm(src_tar, src_file) < 0)
    return -1;

  return 0;
}


int main (int argc, char *argv[])
{
  binary_command cmd =
    {
      CMD_NAME__,
      mv_tar_to_tar,
      mv_ext_to_tar,
      mv_tar_to_ext,
      SUPPORT_OPT__
    };

  return handle_binary_command (cmd, argc, argv);
}
