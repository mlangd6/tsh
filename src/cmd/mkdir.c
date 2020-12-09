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


int mkdir(char *tar_name, char *path_name, char *options)
{
  int length = strlen(path_name);
  if(path_name[length - 1] != '/')
  {
    path_name[length] = '/';
    path_name[length+1] = '\0';
  }
  if(tar_access(tar_name, path_name, F_OK) > 0){
    errno = EEXIST;
    tar_name[strlen(tar_name)] = '/';
    char buf[strlen(tar_name)+34];
    strcpy(buf, "Impossible to create directory \"");
    strcat(buf, tar_name);
    strcat(buf, "\"\0");
    error_cmd(CMD_NAME, buf);
    return EXIT_FAILURE;
  }
  if(tar_add_file(tar_name, NULL, path_name) != 0)
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
