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

static int rm_access_in_existing(char *tar_name, char *filename)
{
  if(strcmp(filename, "\0")==0){
    if(access(tar_name, F_OK) < 0)
      return -1;
  }
  else if(tar_access(tar_name, filename, F_OK) < 0){
    return -1;
  }
  return 0;
}

//check if the access right are good to use rm
static int rm_access_in_writing(char *tar_name, char *filename)
{
  int length = strlen(filename);
  char copy[length+2];
  strcpy(copy, filename);
  char *search = malloc(length);
  if(copy[length-1] == '/')
    copy[length - 1] = '\0';

  search = strrchr(copy, '/');
  if(search != NULL)
  {
    copy[strlen(copy) - strlen(search)+1 ] = '\0';
    if(filename[length-1] == '/' && tar_access(tar_name, copy, W_OK) < 0)
      return -1;
    if(tar_access(tar_name, filename, W_OK) < 0)
      return -1;
  }
  else
  {
    if(access(tar_name, W_OK) < 0)
      return -1;
  }

  return 0;
}

static int is_pwd_prefix(char *tar_name, char *filename)
{
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

  if(is_prefix(nn, env) > 0)
  {
    free(ntn);
    free(ntf);
    free(nn);
    free(env);
    return -1;
  }

  free(ntn);
  free(ntf);
  free(nn);
  free(env);
  return 0;
}

static int get_char()
{
  char c;
  return (read(0, &c, 1)==1?(int)c:EOF);
}

static int prompt_remove(char *tar_name, char *filename)
{
  char a;
  char buf[1024];
  strcpy(buf, CMD_NAME);
  sprintf(buf, "%s : remove \"%s/%s\" which is protected in writing ? : put \'y\' for yes \n", CMD_NAME, tar_name, filename);
  write(STDOUT_FILENO, buf, strlen(buf));
  a = get_char();
  if(a != 'y')
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
    if(tar_rm(tar_name, filename) == -1)
    {
      errno = EINTR;
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
  if(tar_rm(tar_name, filename) == -1)
  {
    errno = EINTR;
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
  char new_filename[strlen(filename)+2];
  strcpy(new_filename, filename);
  //Check if the file isn't prefix on the path of pwd
  if(is_pwd_prefix(tar_name, filename) == -1)
  {
    char msg[strlen(tar_name)+10];
    tar_name[strlen(tar_name)] = '/';
    strcpy(msg, tar_name);
    strcat(msg, ": Impossible to remove, is prefix of pwd");
    error_cmd(CMD_NAME, msg);
    return EXIT_FAILURE;
  }
  //Check if the file is directory in the tar
  if(is_dir(tar_name, filename))
  {
    char *copy_filename = append_slash(filename);
    new_filename[0] = '\0';
    strcpy(new_filename, copy_filename);
    free(copy_filename);
  }

  if(rm_access_in_existing(tar_name, new_filename) == -1)
  {
    tar_name[strlen(tar_name)] = '/';
    error_cmd(CMD_NAME, tar_name);
    return EXIT_FAILURE;
  }

  if(rm_access_in_writing(tar_name, new_filename) == -1){
    if(prompt_remove(tar_name, new_filename) == -1)
      return EXIT_FAILURE;
  }
  return (strchr(options, 'r'))? rm_r(tar_name, new_filename) : rm_(tar_name, new_filename);
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
