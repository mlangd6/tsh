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

#define CMD_NAME "mv"

static void set_cmd_name()
{
  cmd_name_copy[0] = '\0';
  strcpy(cmd_name_copy, "mv");
}

static void set_remove_cmd_name()
{
  cmd_name_remove[0] = '\0';
  strcpy(cmd_name_remove, "mv");
}

static int do_rm(char *src_tar, char *src_file)
{
  set_remove_cmd_name();
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
  set_cmd_name();
  if(cp_tar_to_tar(src_tar, src_file, dest_tar, dest_file, "r") < 0)
    return -1;

  if(do_rm(src_tar, src_file) < 0)
    return -1;

  return 0;
}

int mv_ext_to_tar (char *src_file, char *dest_tar, char *dest_file, char *opt)
{
  set_cmd_name();
  if(cp_ext_to_tar(src_file, dest_tar, dest_file, "r") < 0)
    return -1;

  int i = 0;
  switch(fork())
  {
    case -1:
      i = -1;
      break;
    case 0:
      if(execlp("rm", "rm", "-r", src_file, NULL) < 0)i = -1;
      break;
    default:
      wait(NULL);
  }
  if(i == -1)
  {
    error_cmd(CMD_NAME, src_file);
  }
  return 0;
}

int mv_tar_to_ext(char *src_tar, char *src_file, char *dest_file, char *opt)
{
  set_cmd_name();
  if(cp_tar_to_ext(src_tar, src_file, dest_file, "r") < 0)
    return -1;

  if(do_rm(src_tar, src_file) < 0)
    return -1;

  return 0;
}


int main (int argc, char *argv[])
{
  set_cmd_name();
  binary_command cmd =
    {
      CMD_NAME,
      mv_tar_to_tar,
      mv_ext_to_tar,
      mv_tar_to_ext,
      ""
    };

  return handle_binary_command (cmd, argc, argv);
}
