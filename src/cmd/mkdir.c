//#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "tar.h"
#include "errors.h"
#include "command_handler.h"

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
  if(filename[strlen(filename) - 1] != '/')
    strcat(filename, "/");

  int res_access = access_mkdir(tar_name, filename);
  int len_tar_name = strlen(tar_name);
  if(res_access == -2)
  {
    errno = EEXIST;
    tar_name[len_tar_name] = '/';
    char buf[len_tar_name+27];
    sprintf(buf, "cannot create directory \'%s\'", tar_name);
    error_cmd(CMD_NAME, buf);
    return EXIT_FAILURE;
  }
  if(res_access == -1)
  {
    tar_name[len_tar_name] = '/';
    error_cmd(CMD_NAME, tar_name);
    return EXIT_FAILURE;
  }
  if(tar_add_file(tar_name, NULL, filename) != 0)
  {
    tar_name[len_tar_name] = '/';
    error_cmd(CMD_NAME, tar_name);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}


int main(int argc, char *argv[]){
  command cmd = {
    CMD_NAME,
    mkdir,
    0,
    0,
    ""
  };
  return handle(cmd, argc, argv);
}
