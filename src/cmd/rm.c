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


//check if the access right are good to use rm
static int rm_access(char *tar_name, char *filename)
{
  int length = strlen(filename);
  char copy[length];
  strcpy(copy, filename);
  if(copy[length-1] == '/')
    copy[length - 1] = '\0';
  char *search = malloc(length);
  search = strrchr(copy, '/');
  if(search != NULL)
  {
    copy[length - strlen(search) + 1] = '\0';
    if(tar_access(tar_name, copy, W_OK) < 0)
      return -1;
  }
  else
  {
    if(access(tar_name, W_OK) < 0)
      return -1;
  }
  if(tar_access(tar_name, filename, W_OK) < 0)
    return -1;

  return 0;
}

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
    if((r = tar_rm(tar_name, filename)) == -1)
    {
      errno = EINTR;
      tar_name[strlen(tar_name)] = '/';
      error_cmd(CMD_NAME, tar_name);
      return EXIT_FAILURE;
    }
    if(r == -2)
    {
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
  if((r = tar_rm(tar_name, filename)) == -1)
  {
    errno = EINTR;
    tar_name[strlen(tar_name)] = '/';
    error_cmd(CMD_NAME, tar_name);
    return EXIT_FAILURE;
  }
  if(r == -2)
  {
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

  if(rm_access(tar_name, filename) == -1){
    char a;
    char buf[1024];
    strcpy(buf, CMD_NAME);
    strcat(buf, " : remove \"");
    strcat(buf, tar_name);
    strcat(buf, "/");
    strcat(buf, filename);
    strcat(buf, "\" which is protected in writing ? : put \'y\' for yes \n");
    write(STDOUT_FILENO, buf, strlen(buf));
    a = getchar();
    if(a != 'y')
      return EXIT_FAILURE;
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
