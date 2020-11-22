//#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "tar.h"
//#include "path_lib.h"
#include "errors.h"
#include "command_handler.h"

#define CMD_NAME "mkdir"


int mkdir(char *tar_name, char *path_name, char *options)
{
  printf("Hello\n");
  int length = strlen(path_name);
  if(path_name[length - 1] != '/')
  {
    path_name[length] = '/';
    path_name[length+1] = '\0';
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
/*
int main(int argc, char *argv[]){
  if(argc == 1){
    execlp(CMD_NAME, CMD_NAME, NULL);
  }
  else if(argc == 2){
    char *tar = argv[1];
    char *path = split_tar_abs_path(tar);
    if(is_tar(tar) == 1){
      mktar(tar, path);
    }
    else{
      execvp(CMD_NAME, argv);
    }
  }
  else
  {
    char arg[argc][100];
    for(int i = 0; i < argc; i++){
      for(int j = 0; j < strlen(argv[i]); j++){
        strcpy(arg[i], argv[i]);
      }
    }
    for(int i = 1 ; i < argc; i++)
    {
      char *tar = arg[i];
      char *path = split_tar_abs_path(tar);
      if(is_tar(tar) == 1){
        mktar(tar, path);
      }
      else {
        int f = fork(), w;
        switch(f){
          case -1 :
            perror("fork");
            break;
          case 0 :
            execlp(CMD_NAME, CMD_NAME, arg[i], NULL);
          default :
            wait(&w);
        }
      }
    }
  }
  return 0;
}*/
