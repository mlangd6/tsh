#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "command_handler.h"
#include "utils.h"
#include "tar.h"
#include "errors.h"

#define CMD_NAME "mkdir"

//Check the acces of the file to create and his father, return -2 for the parent problems
//return -1 if the file already exists else 0
static int access_mkdir(char *tar_name, char *filename)
{
  char copy_filename[PATH_MAX];
  strcpy(copy_filename, filename);
  copy_filename[strlen(copy_filename) - 1] = '\0';
  if(tar_access(tar_name, filename, F_OK) > 0 || tar_access(tar_name, copy_filename, F_OK) > 0)
    return -2;

  char *after_last_slash = strrchr(copy_filename, '/');
  if(after_last_slash != NULL)
  {
    copy_filename[strlen(copy_filename) - strlen(after_last_slash) + 1] = '\0';
    if(tar_access(tar_name, copy_filename, W_OK) < 0 || tar_access(tar_name, copy_filename, X_OK) < 0)
      return -1;
  }

  return 0;
}



int mkdir(char *tar_name, char *filename, char *options)
{
  append_slash_filename(filename);

  int res_access = access_mkdir(tar_name, filename);

  if (res_access == -2)
  {
    error (EEXIST, "%s: cannot create directory \'%s/%s\'", CMD_NAME, tar_name, filename);
    return EXIT_FAILURE;
  }

  if (res_access == -1)
  {
    tar_error_cmd (CMD_NAME, tar_name, filename);
    return EXIT_FAILURE;
  }

  if (add_ext_to_tar(tar_name, NULL, filename) != 0)
  {
    tar_error_cmd (CMD_NAME, tar_name, filename);
    return EXIT_FAILURE;
  }

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
