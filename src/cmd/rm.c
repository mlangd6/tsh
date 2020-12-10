#include "tar.h"
#include "errors.h"
#include "command_handler.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define CMD_NAME "rm"
#define SUPPORT_OPT "r"

//"rm ..."
static int rm_(char *tar_name, char *filename)
{
  if(is_dir_name(filename) || strcmp(filename, "\0") == 0)
  {
    errno = EISDIR;
    tar_name[strlen(tar_name)] = '/';
    error_cmd(CMD_NAME, tar_name);
    return EXIT_FAILURE;
  }
  else {
    int r = 0;
    if((r = tar_rm(tar_name, filename)) == -1){
      errno = EINTR;
      tar_name[strlen(tar_name)] = '/';
      error_cmd(CMD_NAME, tar_name);
      return EXIT_FAILURE;
    }
    if(r == -2){
      errno = ENOENT;
      tar_name[strlen(tar_name)] = '/';
      error_cmd(CMD_NAME, tar_name);
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

// "rm -r ..."
static int rm_r(char *tar_name, char *filename)
{
  int r = 0;
  if((r = tar_rm(tar_name, filename)) == -1){
    errno = EINTR;
    tar_name[strlen(tar_name)] = '/';
    error_cmd(CMD_NAME, tar_name);
    return EXIT_FAILURE;
  }
  if(r == -2){
    errno = ENOENT;
    tar_name[strlen(tar_name)] = '/';
    error_cmd(CMD_NAME, tar_name);
    return EXIT_FAILURE;
  }
  if(strcmp(filename, "\0")==0){
    execlp("rm", "rm", tar_name, NULL);
  }
  return EXIT_SUCCESS;
}

int rm(char *tar_name, char *filename, char *options)
{
  //Check if the file isn't prefix on the path of pwd
  char *ntn = malloc(4096);
  strcpy(ntn, tar_name);
  ntn[strlen(ntn)] = '/';
  ntn[strlen(ntn)+1] = '\0';

  char *ntf = malloc(100);
  strcpy(ntf, filename);

  char *nn = malloc(4096);
  strcpy(nn, ntn);
  strcat(nn, ntf);

  char *env = malloc(4096);
  strcpy(env, getenv("PWD"));
  env[strlen(env)] = '/';
  env[strlen(env)+1] = '\0';

  if(strstr(env, nn) != NULL)
  {
    tar_name[strlen(tar_name)] = '/';
    error_cmd(CMD_NAME, tar_name);
    free(ntn);
    free(ntf);
    free(nn);
    return EXIT_FAILURE;
  }

  free(ntn);
  free(ntf);
  free(nn);

  //Check if the file is directory in the tar
  if(is_dir(tar_name, filename))
  {
    int length = strlen(filename);
    if(filename[length - 1] != '/'){
      filename[length] = '/';
      filename[length + 1] = '\0';
    }
  }

  return (strchr(options, 'r'))? rm_r(tar_name, filename) : rm_(tar_name, filename);
}


int main(int argc, char *argv[])
{
  command cmd = {
    CMD_NAME,
    rm,
    0,
    0,
    SUPPORT_OPT
  };
  return handle(cmd, argc, argv);
}
