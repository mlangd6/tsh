#include <stdbool.h>
#include <stdio.h>

#include "command_handler.h"

#define CMD_NAME "cp"
#define SUPPORT_OPT "r"

int cp_tar_to_tar (char *src_tar, char *src_file, char *dest_tar, char *dest_file, char *opt)
{  
  printf("Copying %s/%s (tar) to %s/%s (tar)\n", src_tar, src_file, dest_tar, dest_file);
  
  return 0;
}

int cp_ext_to_tar (char *src_file, char *dest_tar, char *dest_file, char *opt)
{
  printf("Copying %s (ext) to %s/%s (tar)\n", src_file, dest_tar, dest_file);
  
  return 0;
}

int cp_tar_to_ext (char *src_tar, char *src_file, char *dest_file, char *opt)
{
  printf("Copying %s/%s (tar) to %s (ext)\n", src_tar, src_file, dest_file);
  
  return 0;
}
  

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
