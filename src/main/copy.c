#include <stdio.h>
#include <linux/limits.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include "copy.h"
#include "tar.h"
#include "path_lib.h"
#include "utils.h"


static int isdir_err_cp(const char *filename)
{
  char buf[PATH_MAX+154];
  sprintf(buf, "%s : -%s not specified ; omission of the directory \'%s\'\n", CMD_NAME, SUPPORT_OPT, filename);
  write(STDERR_FILENO, buf, strlen(buf));
  return -1;
}

static int is_dir_ext(const char *filename)
{
  struct stat s;
  if (lstat(filename, &s) < 0)
    return -1;

  return (S_ISDIR(s.st_mode));
}

static int dont_exist(const char *filename)
{
  char buf[PATH_MAX+67];
  sprintf(buf, "%s : Impossible to evaluate \'%s\': No file or directory of this type\n", CMD_NAME, filename);
  write(STDERR_FILENO, buf, strlen(buf));
  return -1;
}

static int already_exist(const char *src, const char *dest)
{
  char buf[2*PATH_MAX+37];
  sprintf(buf, "%s: \'%s\' and \'%s\' identify the same file\n", CMD_NAME, src, dest);
  write(STDERR_FILENO, buf, strlen(buf));
  return -1;
}

static int exist(const char *tar_name, const char *filename, int src_or_dest)
{
  if(tar_access(tar_name, filename, F_OK) > 0)
  {
    if(!src_or_dest)
      return -1;
    return 0;
  }
  if(!src_or_dest)
    return 0;
  return -1;
}

static int exist_ext(const char *filename, int src_or_dest)
{
  if(access(filename, F_OK) > 0)
  {
      if(!src_or_dest)
        return -1;
      return 0;
  }
  if(!src_or_dest)
    return 0;
  return -1;
}
/*
static char *last_word(const char *file)
{
  char *sear = strrchr(file);
  if(sear[1] == '\0')
  {

  }
}*/
static int nb_of_words(char *str)
{
  int res = 1;
  int len = strlen(str);
  for(int i = 1; i < len-1; i++){
    if(str[i] == '/' && str[i+1] != '\0')res++;
  }
  return res;
}

static int cp_without_option(char *src_tar, char *src_file, char *dest_tar, char *dest_file)
{
  if(is_dir(src_tar, src_file))
  {
    char buf[PATH_MAX];
    sprintf(buf, "%s/%s", src_tar, src_file);
    return isdir_err_cp(buf);
  }
  if(exist(src_tar, src_file, 1) < 0){
    char buf[PATH_MAX];
    sprintf(buf, "%s/%s", src_tar, src_file);
    return dont_exist(buf);
  }
  if(is_dir(dest_tar, dest_file))
  {
    char *buf = malloc(100);
    if(nb_of_words(src_file) > 1){
      char src_file_copy[100];
      strcpy(src_file_copy, src_file);
      char *tmp = end_of_path(src_file_copy);
      strcpy(buf, tmp);
      free(tmp);
    }
    else
    {
      strcpy(buf, src_file);
    }
    char buf2[PATH_MAX];
    if(!is_empty_string(dest_file))
    {
      if(dest_file[strlen(dest_file) - 1] == '/')
        dest_file[strlen(dest_file) - 1] = '\0';
      sprintf(buf2, "%s/%s", dest_file, buf);
    }
    else
      sprintf(buf2, "%s", buf);
    if(exist(dest_tar, buf2, 0) > 0)
    {
      if(tar_rm(dest_tar, buf2) < 0)
      {
        write(STDERR_FILENO, "cp: Problems on removing the file of the same name\n", 51);
        free(buf);
        return -1;
      }
    }

    if(add_tar_to_tar(src_tar, dest_tar, src_file, buf2) < 0)
    {
      char err[33];
      sprintf(err, "%s: Problems at the add of file\n", CMD_NAME);
      write(STDERR_FILENO, err, strlen(err));
      free(buf);
      return -1;
    }
    free(buf);
  }
  else
  {
    if(exist(dest_tar, dest_file, 0) < 0)
    {
      if(tar_rm(dest_tar, dest_file) < 0)
      {
        write(STDERR_FILENO, "cp: Problems on removing the file of the same name\n", 51);
        return -1;
      }
    }
    if(add_tar_to_tar(src_tar, dest_tar, src_file, dest_file) < 0)
    {
      char err[33];
      sprintf(err, "%s: Problems at the add of file\n", CMD_NAME);
      write(STDERR_FILENO, err, strlen(err));
      return -1;
    }
  }
  return 0;
}

int cp_tar_to_tar (char *src_tar, char *src_file, char *dest_tar, char *dest_file, char *opt)
{
  if(strcmp(dest_tar, src_tar) == 0 && strcmp(src_file, dest_file) == 0)
  {
    char src[PATH_MAX];
    char dest[PATH_MAX];
    sprintf(src, "%s/%s", src_tar, src_file);
    sprintf(dest, "%s/%s", dest_tar, dest_file);
    return already_exist(src, dest);
  }
  if(is_empty_string(opt))
  {
    return cp_without_option(src_tar, src_file, dest_tar, dest_file);
  }
  else{
    add_tar_to_tar_rec(src_tar, dest_tar, src_file, dest_file);
  }
  return 0;
}


static int cp_without_option_ext_to_tar(char *src_file, char *dest_tar, char *dest_file)
{
  if(is_dir_ext(src_file))
    return isdir_err_cp(src_file);
  if(exist_ext(src_file, 0) < 0)
    return dont_exist(src_file);
  if(is_dir(dest_tar, dest_file))
  {
    char src_file_copy[100];
    strcpy(src_file_copy, src_file);
    char *buf = end_of_path(src_file_copy);

    char buf2[PATH_MAX];
    if(!is_empty_string(dest_file))
    {
      if(dest_file[strlen(dest_file) - 1] == '/')
        dest_file[strlen(dest_file) - 1] = '\0';
      sprintf(buf2, "%s/%s", dest_file, buf);
    }
    else
      sprintf(buf2, "%s", buf);
    if(exist(dest_tar, buf2, 0) > 0){
      if(tar_rm(dest_tar, buf2) < 0)
      {
        write(STDERR_FILENO, "cp: Problems on removing the file of the same name\n", 51);
        free(buf);
        return -1;
      }
    }
    if(add_ext_to_tar(dest_tar, src_file, buf2) < 0)
    {
      char err[33];
      sprintf(err, "%s: Problems at the add of file\n", CMD_NAME);
      write(STDERR_FILENO, err, strlen(err));
      free(buf);
      return -1;
    }
    free(buf);
  }
  else
  {
    if(exist(dest_tar, dest_file, 0) < 0)
    {
      if(tar_rm(dest_tar, dest_file) < 0)
      {
        write(STDERR_FILENO, "cp: Problems on removing the file of the same name\n", 51);
        return -1;
      }
    }
    if(add_ext_to_tar(dest_tar, src_file, dest_file) < 0)
    {
      char err[33];
      sprintf(err, "%s: Problems at the add of file\n", CMD_NAME);
      write(STDERR_FILENO, err, strlen(err));
      return -1;
    }
  }
  return 0;
}

int cp_ext_to_tar (char *src_file, char *dest_tar, char *dest_file, char *opt)
{
  if(is_empty_string(opt))
  {
    return cp_without_option_ext_to_tar(src_file, dest_tar, dest_file);
  }

  else
  {
    add_ext_to_tar_rec(dest_tar, src_file, dest_file, 0);
  }

  return 0;
}

int cp_tar_to_ext (char *src_tar, char *src_file, char *dest_file, char *opt)
{
  printf("Copying %s/%s (tar) to %s (ext)\n", src_tar, src_file, dest_file);

  return 0;
}
