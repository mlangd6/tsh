#include "tar.h"
#include "errors.h"
#include "command_handler.h"
#include "utils.h"
#include "path_lib.h"
#include "remove.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

char cmd_name_remove[3];

void set_remove_cmd_name(const char *str)
{
  cmd_name_remove[0] = '\0';
  strcpy(cmd_name_remove, str);
}

static int rm_access_in_existing(const char *tar_name, const char *filename)
{
  if(is_empty_string(filename)){
    if(access(tar_name, F_OK) < 0)
      return -1;
  }
  else if(tar_access(tar_name, filename, F_OK) < 0){
    return -1;
  }
  return 0;
}

//check if the access right are good to use rm
static int rm_access_in_writing(const char *tar_name, const char *filename)
{
  int len_file = strlen(filename);
  char copy_filename[len_file+2];
  strcpy(copy_filename, filename);
  if(len_file > 0 && copy_filename[len_file-1] == '/')
    copy_filename[len_file - 1] = '\0';
  char *last_slash_of_copy = strrchr(copy_filename, '/');

  if(last_slash_of_copy != NULL)
  {
    int a = strlen(last_slash_of_copy);
    copy_filename[strlen(copy_filename) - a + 1 ] = '\0';
    if(filename[len_file-1] == '/' && tar_access(tar_name, copy_filename, W_OK) < 0){
      return -1;
    }
    if(tar_access(tar_name, filename, W_OK) < 0){
      return -1;
    }
  }

  return 0;
}

static int get_char()
{
  char c[2];
  return (read(STDIN_FILENO, &c, 2)==1?(int)c[0]:EOF);
}

static int prompt_remove(char *tar_name, char *filename)
{
  char a;
  char buf[1024];
  strcpy(buf, cmd_name_remove);
  sprintf(buf, "%s : remove \"%s/%s\" which is protected in writing ? : put \'y\' for yes \n", cmd_name_remove, tar_name, filename);
  write(STDOUT_FILENO, buf, strlen(buf));
  a = get_char();
  if(a != 'y')
    return -1;
  return 0;
}

//"rm ..."
static int rm_(char *tar_name, char *filename)
{
  if(is_dir_name(filename) || is_empty_string(filename))
  {
    errno = EISDIR;
    tar_error_cmd (cmd_name_remove, tar_name, filename);
    return EXIT_FAILURE;
  }
  else {
    if(tar_rm(tar_name, filename) == -1)
    {
      errno = EINTR;
      tar_error_cmd (cmd_name_remove, tar_name, filename);
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
    tar_error_cmd (cmd_name_remove, tar_name, filename);
    return EXIT_FAILURE;
  }

  if(is_empty_string(filename)){
    execlp("rm", "rm", tar_name, NULL);
  }

  return EXIT_SUCCESS;
}


int rm(char *tar_name, char *filename, char *options)
{

  char new_filename[PATH_MAX];
  strcpy(new_filename, filename);
  //Check if the file isn't prefix on the path of pwd

  if(is_pwd_prefix(tar_name, filename) == -1)
  {
    error (0, "%s: %s/%s: Impossible to remove, is prefix of pwd\n", cmd_name_remove, tar_name, filename);
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
    tar_error_cmd (cmd_name_remove, tar_name, filename);
    return EXIT_FAILURE;
  }

  if(rm_access_in_writing(tar_name, new_filename) == -1){
    if(prompt_remove(tar_name, new_filename) == -1)
      return EXIT_FAILURE;
  }

  return (strchr(options, 'r'))? rm_r(tar_name, new_filename) : rm_(tar_name, new_filename);
}
