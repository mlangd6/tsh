#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "copy.h"
#include "errors.h"
#include "path_lib.h"
#include "tar.h"
#include "utils.h"


static int is_dir_ext(const char *filename);
static int exist(const char *tar_name, const char *filename, int src_or_dest);
static int exist_ext(const char *filename, int src_or_dest);
static int nb_of_words(char *str);
static int has_rights_src(char *src_tar, char *src_file);
static int has_rights_dest(char *dest_tar, char *dest_file);
static int has_rights_src_ext(char *src_file);
static int has_rights_dest_ext(char *dest_file);
static void end_of_path_filename(char *path, char *the_end_of_path);
static int when_is_dir_dest(char *src_tar, char *src_file, char *dest_tar, char *dest_file);
static int cp_ttt_without_r(char *src_tar, char *src_file, char *dest_tar, char *dest_file);
static int cp_r_ttt(char *src_tar, char *src_file, char *dest_tar, char *dest_file);
static int cp_ett_without_r(char *src_file, char *dest_tar, char *dest_file);
static int cp_r_ett(char *src_file, char *dest_tar, char *dest_file);
static int rm_touch(char *filename, int rm);
static int error_rm_touch(int a);
static int exec_cp(char *src, char *dest);
static int cp_tte_without_r(char *src_tar, char *src_file, char *dest_file);
static int cp_r_tte(char *src_tar, char *src_file, char *dest_file);

char cmd_name_copy[3];


void set_cmd_name(char *str)
{
  cmd_name_copy[0] = '\0';
  strcpy(cmd_name_copy, str);
}



static int is_dir_ext(const char *filename)
{
  struct stat s;
  if (lstat(filename, &s) < 0)
    return 0;

  return (S_ISDIR(s.st_mode));
}


static int exist(const char *tar_name, const char *filename, int src_or_dest)
{
  if(tar_access(tar_name, filename, F_OK) > 0)
  {
    if(src_or_dest == 0)
      return -1;
    return 0;
  }
  if(src_or_dest == 0)
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


static int nb_of_words(char *str)
{
  int res = 1;
  int len = strlen(str);
  for(int i = 1; i < len-1; i++){
    if(str[i] == '/' && str[i+1] != '\0')res++;
  }
  return res;
}

static int has_rights_src(char *src_tar, char *src_file)
{
  if(tar_access(src_tar, src_file, R_OK) < 0)
  {
    errno = EACCES;
    error(errno, "%s: impossible to open \'%s/%s\' in reading", cmd_name_copy, src_tar, src_file);
    return -1;
  }
  return 0;
}

static int has_rights_dest(char *dest_tar, char *dest_file)
{
  if(is_empty_string(dest_file))return 0;
  if(tar_access(dest_tar, dest_file, X_OK) < 0)
  {
    errno = EACCES;
    error(errno, "%s: can't access \'%s/%s\'", cmd_name_copy, dest_tar, dest_file);
    return -1;
  }
  if(tar_access(dest_tar, dest_file, W_OK) < 0)
  {
    errno = EACCES;
    error(errno, "%s: impossible to create the standard file \'%s/%s\'", cmd_name_copy, dest_tar, dest_file);
    return -1;
  }
  return 0;
}

static int has_rights_src_ext(char *src_file)
{
  if(access(src_file, R_OK) < 0)
  {
    errno = EACCES;
    error(errno, "%s: impossible to open \'%s\' in reading", cmd_name_copy, src_file);
    return -1;
  }
  return 0;
}

static int has_rights_dest_ext(char *dest_file)
{
  if(access(dest_file, X_OK) < 0)
  {
    errno = EACCES;
    error(errno, "can't access \'%s\'", cmd_name_copy, dest_file);
    return -1;
  }
  if(access(dest_file, W_OK) < 0)
  {
    errno = EACCES;
    error(errno, "%s: impossible to create the standard file \'%s\'", cmd_name_copy, dest_file);
    return -1;
  }
  return 0;
}

static void end_of_path_filename(char *path, char *the_end_of_path)
{
  char *end_ofpath = end_of_path(path);
  strcpy(the_end_of_path, end_ofpath);
  free(end_ofpath);
}

static int when_is_dir_dest(char *src_tar, char *src_file, char *dest_tar, char *dest_file)
{
  char buf[100];
  if(nb_of_words(src_file) > 1){
    char src_file_copy[100];
    strcpy(src_file_copy, src_file);
    end_of_path_filename(src_file_copy, buf);
  }
  else
  {
    strcpy(buf, src_file);
  }
  char buf2[PATH_MAX];
  if(!is_empty_string(dest_file)){
    append_slash_filename(dest_file);
    sprintf(buf2, "%s%s", dest_file, buf);
  }
  else
    sprintf(buf2, "%s", buf);
  if(has_rights_dest(dest_tar, dest_file) < 0)
    return -1;
  if(exist(dest_tar, buf2, 1) == 0)
  {
    if(tar_rm(dest_tar, buf2) < 0)
    {
      error(0, "%s: Problems on removing the file of the same name\n", cmd_name_copy);
      return -1;
    }
  }
  if(add_tar_to_tar(src_tar, dest_tar, src_file, buf2) < 0)
  {
    error(0, "%s: Problems at the add of file\n", cmd_name_copy);
    return -1;
  }
  return 0;
}

static int cp_ttt_without_r(char *src_tar, char *src_file, char *dest_tar, char *dest_file)
{
  if(is_dir(src_tar, src_file))
  {
    error(0, "%s : -r not specified ; omission of the directory \'%s/%s\'\n", cmd_name_copy, src_tar, src_file);
    return -1;
  }
  if(exist(src_tar, src_file, 1) < 0)
  {
    errno = ENOENT;
    error(errno, "%s : Impossible to evaluate \'%s/%s\'", cmd_name_copy, src_tar, src_file);
    return -1;
  }
  if(has_rights_src(src_tar, src_file) < 0)
    return -1;
  if(is_dir(dest_tar, dest_file))
  {
    return (when_is_dir_dest(src_tar, src_file, dest_tar, dest_file) < 0)?-1:0;
  }
  else
  {
    if(exist(dest_tar, dest_file, 1) == 0)
    {
      if(tar_rm(dest_tar, dest_file) < 0)
      {
        error(0, "%s: Problems on removing the file of the same name\n", cmd_name_copy);
        return -1;
      }
    }
    if(add_tar_to_tar(src_tar, dest_tar, src_file, dest_file) < 0)
    {
      error(0, "%s: Problems at the add of file\n", cmd_name_copy);
      return -1;
    }
  }
  return 0;
}

static int cp_r_ttt(char *src_tar, char *src_file, char *dest_tar, char *dest_file)
{
  if(!is_dir(src_tar, src_file))
    return cp_ttt_without_r(src_tar, src_file, dest_tar, dest_file);
  if(is_empty_string(dest_file))
  {
    char dest_tar_copy[PATH_MAX];
    strcpy(dest_tar_copy, dest_tar);
    if(is_empty_string(src_file))
    {
      char str[100];
      strcpy(str, src_tar);
      if(nb_of_words(src_tar) > 1)
        end_of_path_filename(str, dest_file);
      else
        strcpy(dest_file, src_tar);

      append_slash_filename(dest_file);
      add_tar_to_tar_rec(src_tar, dest_tar_copy, "\0", dest_file);
      add_ext_to_tar(dest_tar_copy, NULL, dest_file);
      return 0;
    }
    char str[100];
    strcpy(str, src_file);
    if(nb_of_words(src_file) > 1)
      end_of_path_filename(str, dest_file);
    else
      strcpy(dest_file, src_file);

    append_slash_filename(dest_file);
    append_slash_filename(src_file);

    return add_tar_to_tar_rec(src_tar, dest_tar_copy, src_file, dest_file);
  }
  int new = 0;
  if(!is_dir(dest_tar, dest_file))
  {
    if(exist(dest_tar, dest_file, 1) == 0)
    {
      error(0, "%s : Impossible to break the not-directory \'%s/%s\' by the directory \'%s/%s\'\n", cmd_name_copy, src_tar, src_file, dest_tar, dest_file);
      return -1;
    }
    append_slash_filename(dest_file);
    new = 1;
  }
  else
  {
    append_slash_filename(dest_file);
    if(has_rights_dest(dest_tar, dest_file) < 0)
      return -1;
  }
  if(is_empty_string(src_file) && new == 1)
  {
    add_tar_to_tar_rec(src_tar, dest_tar, src_file, dest_file);
    add_ext_to_tar(dest_tar, NULL, dest_file);
    return 0;
  }
  if(!is_empty_string(src_file))
  {
    if(is_dir(src_tar, src_file))
      append_slash_filename(src_file);
    if(exist(src_tar, src_file, 1) < 0)
    {
      errno = ENOENT;
      error(0, "%s : Impossible to evaluate \'%s/%s\'", cmd_name_copy, src_tar, src_file);
      return -1;
    }
    if(has_rights_src(src_tar, src_file) < 0){
      return -1;
    }
  }
  if(new == 1){
    if(add_tar_to_tar_rec(src_tar, dest_tar, src_file, dest_file) < 0)
    {
      error(0, "%s: Problems at the add of file\n", cmd_name_copy);
      return -1;
    }
  }
  else
  {
    char buf[100];
    char buf2[PATH_MAX];
    if(!is_empty_string(src_file))
    {
      if(nb_of_words(src_file) > 1){
        char src_file_copy[100];
        strcpy(src_file_copy, src_file);
        end_of_path_filename(src_file_copy, buf);
      }
      else
        strcpy(buf, src_file);
      if(!is_empty_string(dest_file))
      {
        if(dest_file[strlen(dest_file) - 1] == '/')
          dest_file[strlen(dest_file) - 1] = '\0';
        sprintf(buf2, "%s/%s", dest_file, buf);
      }
      else
        sprintf(buf2, "%s", buf);
    }
    else
    {
      if(nb_of_words(src_tar) > 1){
        char src_tar_copy[PATH_MAX];
        strcpy(src_tar_copy, src_tar);
        end_of_path_filename(src_tar_copy, buf);
      }
      else
        strcpy(buf, src_tar);
      sprintf(buf2, "%s%s/", dest_file, buf);
    }
    if(exist(dest_tar, buf2, 1) == 0)
      return -1;

    if(add_tar_to_tar_rec(src_tar, dest_tar, src_file, buf2) < 0)
    {
      error(0, "%s: Problems at the add of file\n", cmd_name_copy);
      return -1;
    }
    if(is_empty_string(src_file))
    {
      if(add_ext_to_tar(dest_tar, NULL, buf2) < 0)
      {
        error(0, "%s: Problems at the add of file\n", cmd_name_copy);
        return -1;
      }
    }
  }
  return 0;
}

int cp_tar_to_tar(char *src_tar, char *src_file, char *dest_tar, char *dest_file, char *opt)
{
  if(strcmp(dest_tar, src_tar) == 0 && strcmp(src_file, dest_file) == 0)
  {
    char src[PATH_MAX];
    char dest[PATH_MAX];
    sprintf(src, "%s/%s", src_tar, src_file);
    sprintf(dest, "%s/%s", dest_tar, dest_file);
    if(is_empty_string(opt))
    {
      error(0, "%s: \'%s\' and \'%s\' identify the same file\n", cmd_name_copy, src_tar, src_file, dest_tar, dest_file);
      return -1;
    }
    else
    {
      error(0, "%s: impossible to create the directory (\'%s\') in himself (\'%s/%s\')\n", cmd_name_copy, src_file, src_file, src_file);
      return -1;
    }
  }
  if(is_empty_string(opt))
  {
    return cp_ttt_without_r(src_tar, src_file, dest_tar, dest_file);
  }
  else
  {
    return cp_r_ttt(src_tar, src_file, dest_tar, dest_file);
  }
  return 0;
}


static int cp_ett_without_r(char *src_file, char *dest_tar, char *dest_file)
{
  if(is_dir_ext(src_file))
  {
    error(0, "%s : -r not specified ; omission of the directory \'%s\'\n", cmd_name_copy, src_file);
    return -1;
  }
  if(exist_ext(src_file, 1) == 0)
  {
    errno = ENOENT;
    error(errno, "%s : Impossible to evaluate \'%s\'", cmd_name_copy, src_file);
    return -1;
  }
  if(has_rights_src_ext(src_file) < 0)
    return -1;

  if(is_dir(dest_tar, dest_file) && !is_empty_string(dest_file))
    append_slash_filename(dest_file);
  if(exist(dest_tar, dest_file, 1) == 0){
    if(has_rights_dest(dest_tar, dest_file) < 0)
      return -1;
  }
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
    if(exist(dest_tar, buf2, 1) == 0){
      if(tar_rm(dest_tar, buf2) < 0)
      {
        error(0, "%s: Problems on removing the file of the same name\n", cmd_name_copy);
        free(buf);
        return -1;
      }
    }
    if(add_ext_to_tar(dest_tar, src_file, buf2) < 0)
    {
      error(0, "%s: Problems at the add of file\n", cmd_name_copy);
      free(buf);
      return -1;
    }
    free(buf);
  }
  else
  {

    if(exist(dest_tar, dest_file, 1) == 0)
    {
      if(tar_rm(dest_tar, dest_file) < 0)
      {
        error(0, "%s: Problems on removing the file of the same name\n", cmd_name_copy);
        return -1;
      }
    }
    if(add_ext_to_tar(dest_tar, src_file, dest_file) < 0)
    {
      error(0, "%s: Problems at the add of file\n", cmd_name_copy);
      return -1;
    }
  }
  return 0;
}

static int cp_r_ett(char *src_file, char *dest_tar, char *dest_file)
{
  if(!is_dir_ext(src_file))return cp_ett_without_r(src_file, dest_tar, dest_file);
  if(is_empty_string(dest_file))
  {
    char dest_tar_copy[PATH_MAX];
    strcpy(dest_tar_copy, dest_tar);
    char str[100];
    strcpy(str, src_file);
    if(nb_of_words(src_file) > 1)
      end_of_path_filename(str, dest_file);
    else
      strcpy(dest_file, src_file);

    append_slash_filename(dest_file);
    append_slash_filename(src_file);

    return add_ext_to_tar_rec(dest_tar_copy, src_file, dest_file, 0);
  }

  int new = 0;
  if(!is_dir(dest_tar, dest_file))
  {
    if(exist(dest_tar, dest_file, 1) == 0)
    {
      error(0, "%s : Impossible to break the not-directory \'%s\' by the directory \'%s/%s\'\n", cmd_name_copy, src_file, dest_tar, dest_file);
      return -1;
    }
    append_slash_filename(dest_file);
    new = 1;
  }
  else
  {
    append_slash_filename(dest_file);
    if(has_rights_dest(dest_tar, dest_file) < 0)
      return -1;
  }
  if(is_dir_ext(src_file))
    append_slash_filename(src_file);

  if(exist_ext(src_file, 0) == 0)
  {
    errno = ENOENT;
    error(errno, "%s : Impossible to evaluate \'%s\'", cmd_name_copy, src_file);
    return -1;
  }
  if(has_rights_src_ext(src_file) < 0){
    return -1;
  }
  if(new == 1){
    if(add_ext_to_tar_rec(dest_tar, src_file, dest_file, 0) < 0)
    {
      error(0, "%s: Problems at the add of file\n", cmd_name_copy);
      return -1;
    }
  }
  else
  {
    char *buf = malloc(100);
    if(nb_of_words(src_file) > 1){
      char src_file_copy[100];
      strcpy(src_file_copy, src_file);
      end_of_path_filename(src_file_copy, buf);
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
    if(exist(dest_tar, buf2, 1) == 0)
    {
      free(buf);
      return -1;
    }
    if(add_ext_to_tar_rec(dest_tar, src_file, buf2, 0) < 0)
    {
      error(0, "%s: Problems at the add of file\n", cmd_name_copy);
      free(buf);
      return -1;
    }
    free(buf);
  }
  return 0;
}

int cp_ext_to_tar (char *src_file, char *dest_tar, char *dest_file, char *opt)
{
  if(is_empty_string(opt))
  {
    return cp_ett_without_r(src_file, dest_tar, dest_file);
  }

  else
  {
    return cp_r_ett(src_file, dest_tar, dest_file);
  }

  return 0;
}

static int rm_touch(char *filename, int rm)//1 = rm et 0 = touch et 2 == mkdir
{
  int a = 0;
  if(rm == 1)
  {
    switch(fork())
    {
      case -1:
        a = -1;
        exit(2);
        break;
      case 0:
        if(execlp("rm", "rm", filename, NULL) < 0)a = -2;
        break;
      default:
        wait(NULL);
    }
  }
  else if(rm == 2)
  {
    switch(fork())
    {
      case -1:
        a = -1;
        exit(2);
        break;
      case 0:
        if(execlp("mkdir", "mkdir", filename, NULL) < 0)a = -2;
        break;
      default:
        wait(NULL);
    }
  }
  else
  {
    switch(fork())
    {
      case -1:
        a = -1;
        exit(2);
        break;
      case 0:
        if(execlp("touch", "touch", filename, NULL) < 0)a = -2;
        break;
      default:
        wait(NULL);
    }
  }
  return a;
}

static int error_rm_touch(int a)
{
  if(a == -1)
  {
    error(errno, "Error Fork()");
    return -1;
  }
  if(a == -2)
  {
    error(errno, "Error Execlp()");
    return -1;
  }
  return -1;
}

static int exec_cp(char *src, char *dest)
{
  int a = 0;
  switch(fork())
  {
    case -1:
      a = -1;
      exit(2);
      break;
    case 0:
      if(execlp("cp", "cp", src, dest, NULL) < 0)a = -1;
      break;
    default:
      wait(NULL);
  }
  return a;
}

static int cp_tte_without_r(char *src_tar, char *src_file, char *dest_file)
{
  if(is_dir(src_tar, src_file))
  {
    error(0, "%s : -r not specified ; omission of the directory \'%s/%s\'\n", cmd_name_copy, src_tar, src_file);
    return -1;
  }
  if(exist(src_tar, src_file, 1) == -1)
  {
    errno = ENOENT;
    error(errno, "%s : Impossible to evaluate \'%s/%s\'", cmd_name_copy, src_tar, src_file);
    return -1;
  }
  if(has_rights_src(src_tar, src_file) < 0)
    return -1;


  if(exist_ext(dest_file, 1)==0)
  {
    if(has_rights_dest_ext(dest_file) < 0)
      return -1;
  }
  if(is_dir_ext(dest_file) != 0)
  {
    char src_file_copy[PATH_MAX];
    if(is_empty_string(src_file))
      strcpy(src_file_copy, src_tar);
    else
      sprintf(src_file_copy, "%s/%s", src_tar, src_file);
    char *buf = end_of_path(src_file_copy);

    char buf2[PATH_MAX];

    if(dest_file[strlen(dest_file) - 1] == '/')
      dest_file[strlen(dest_file) - 1] = '\0';
    sprintf(buf2, "%s/%s", dest_file, buf);

    if(exist_ext(buf2, 1) == 0){
      int a = rm_touch(buf2, 1);
      if(a < 0)
        return error_rm_touch(a);
    }
    int a = rm_touch(buf2, 0);
    if(a < 0)
      return error_rm_touch(a);

    int fd = open(buf2, O_RDWR);
    if(fd < 0)
    {
      error_cmd(cmd_name_copy, buf2);
      return -1;
    }
    if(tar_cp_file(src_tar, src_file, fd) < 0)
    {
      error(0, "%s: Problems at the add of file\n", cmd_name_copy);
      free(buf);
      return -1;
    }
    free(buf);
  }
  else
  {
    if(exist_ext(dest_file, 1) == 0){
      int a = rm_touch(dest_file, 1);
      if(a < 0)
        return error_rm_touch(a);
    }
    int a = rm_touch(dest_file, 0);
    if(a < 0)
      return error_rm_touch(a);

    int fd = open(dest_file, O_RDWR);
    if(fd < 0)
    {
      error_cmd(cmd_name_copy, dest_file);
      return -1;
    }
    if(tar_cp_file(src_tar, src_file, fd) < 0)
    {
      error(0, "%s: Problems at the add of file\n", cmd_name_copy);
      return -1;
    }
  }
  return 0;
}

static int cp_r_tte(char *src_tar, char *src_file, char *dest_file)
{
  if(!is_dir(src_tar, src_file))return cp_tte_without_r(src_tar, src_file, dest_file);

  if(is_empty_string(src_file))
  {
    return exec_cp(src_tar, dest_file);
  }
  int new = 0;
  if(!is_dir_ext(dest_file))
  {
    if(exist_ext(dest_file, 1) == 0)
    {
      error(0, "%s : Impossible to break the not-directory \'%s/%s\' by the directory \'%s\'\n", cmd_name_copy, src_tar, src_file, dest_file);
      return -1;
    }
    append_slash_filename(dest_file);
    new = 1;
  }
  else
  {
    append_slash_filename(dest_file);
    if(has_rights_dest_ext(dest_file) < 0)
      return -1;
  }

  if(is_dir(src_tar, src_file))
    append_slash_filename(src_file);

  if(exist(src_tar, src_file, 0) == 0)
  {
    errno = ENOENT;
    error(errno, "%s : Impossible to evaluate \'%s/%s\'", cmd_name_copy, src_tar, src_file);
    return -1;
  }
  if(has_rights_src(src_tar, src_file) < 0){
    return -1;
  }
  if(new == 1){
    dest_file[strlen(dest_file) - 1 ] = '\0';
    char *buf = strrchr(dest_file, '/');
    char tmp[100];
    strcpy(tmp, buf+1);
    if(buf[0] != '\0')buf[0] = '\0';
    if(tar_extract(src_tar, src_file, dest_file) < 0)
    {
      error(0, "%s: Problems at the add of file\n", cmd_name_copy);
      return -1;
    }

    char buf2[PATH_MAX];
    sprintf(buf2, "%s/%s", dest_file, tmp);
    char buf3[PATH_MAX];
    sprintf(buf3, "%s/%s", dest_file, src_file);
    if(rename(buf3, buf2) != 0)return -1;
  }
  else
  {
    if(tar_extract(src_tar, src_file, dest_file) < 0)
    {
      error(0, "%s: Problems at the add of file\n", cmd_name_copy);
      return -1;
    }
  }
  return 0;
}

int cp_tar_to_ext (char *src_tar, char *src_file, char *dest_file, char *opt)
{
  if(is_empty_string(opt))
  {
    return cp_tte_without_r(src_tar, src_file, dest_file);
  }
  else
  {
    return cp_r_tte(src_tar, src_file, dest_file);
  }

  return 0;
}
