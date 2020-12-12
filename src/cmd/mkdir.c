//#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "tar.h"
#include "errors.h"
#include "command_handler.h"

#define CMD_NAME "mkdir"

//Check the acces of the file to create and his father, return -2 for the parent problems
//return -1 if the file already exists else 0
static int access_mkdir(char *tar_name, char *filename)
{
  if(tar_access(tar_name, filename, F_OK) > 0)
  {
    return -2;
  }
  char tmp[strlen(filename)];
  strcpy(tmp, filename);
  tmp[strlen(tmp) - 1] = '\0';
  char *search = malloc(100);
  search = strrchr(tmp, '/');
  if(search != NULL)
  {
    tmp[strlen(tmp) - strlen(search) + 1] = '\0';
    if(tar_access(tar_name, tmp, W_OK) < 0 || tar_access(tar_name, tmp, X_OK) < 0)
      return -1;
  }
  else
  {
    if(access(tar_name, W_OK) < 0 || access(tar_name, X_OK) < 0)
      return -1;
  }
  return 0;
}



int mkdir(char *tar_name, char *filename, char *options)
{
  int length = strlen(filename);
  if(filename[length - 1] != '/')
  {
    filename[length] = '/';
    filename[length+1] = '\0';
  }
  int i = access_mkdir(tar_name, filename);
  if(i == -2)
  {
    errno = EEXIST;
    tar_name[strlen(tar_name)] = '/';
    char buf[strlen(tar_name)+34];
    strcpy(buf, "Impossible to create directory \"");
    strcat(buf, tar_name);
    strcat(buf, "\"\0");
    error_cmd(CMD_NAME, buf);
    return EXIT_FAILURE;
  }
  if(i == -1)
  {
    tar_name[strlen(tar_name)] = '/';
    error_cmd(CMD_NAME, tar_name);
    return EXIT_FAILURE;
  }
  if(tar_add_file(tar_name, NULL, filename) != 0)
  {
    tar_name[strlen(tar_name)] = '/';
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
