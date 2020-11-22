/*Problems :
    Quand on declare un fichier comme un répertoire la suppression est rendu impossible par la gestion des chemins et non pas par la fonction rm
    Meme probleme quand on appelle un fichier sans le dernier /
*/

#include "tar.h"
#include "errors.h"
#include "command_handler.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define CMD_NAME "rm"
#define SUPPORT_OPT "r"

static int rm_(char *tar_name, char *filename)
{
  printf("rm\n");
  if(is_dir_name(filename))
  {
    tar_name[strlen(tar_name)] = '/';
    error_cmd(CMD_NAME, tar_name);
    return EXIT_FAILURE;
  }
  else {
    if(tar_rm(tar_name, filename) == -1){
      tar_name[strlen(tar_name)] = '/';
      error_cmd(CMD_NAME, tar_name);
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

static int rm_r(char *tar_name, char *filename)
{
  printf("rm -r\n");
  if(is_dir_name(filename)){
    int length = strlen(filename);
    if(filename[length - 1] != '/')
    {
      filename[length] = '/';
      filename[length+1] = '\0';
    }
  }
  if(tar_rm(tar_name, filename) == -1){
    tar_name[strlen(tar_name)] = '/';
    error_cmd(CMD_NAME, tar_name);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int rm(char *tar_name, char *filename, char *options)
{
  printf("rm\n");
  return (strchr(options, 'r'))? rm_r(tar_name, filename) : rm_(tar_name, filename);
}


int main(int argc, char *argv[])
{
  printf("main\n");
  command cmd = {
    CMD_NAME,
    rm,
    1,
    1,
    SUPPORT_OPT
  };
  return handle(cmd, argc, argv);
}
