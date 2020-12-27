#include <stdio.h>
#include <linux/limits.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "copy.h"
#include "tar.h"
#include "path_lib.h"
#include "utils.h"
#include "errors.h"


static int isfile_err_cp(const char *filename, const char *dest)
{
  char buf[PATH_MAX+154];
  sprintf(buf, "%s : Impossible to break the not-directory \'%s\' by the directory \'%s\'\n", CMD_NAME, filename, dest);
  write(STDERR_FILENO, buf, strlen(buf));
  return -1;
}

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
    return 0;

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

static int has_rights_src(char *src_tar, char *src_file)
{
  if(tar_access(src_tar, src_file, R_OK) < 0)
  {
    char buf[PATH_MAX+100];
    sprintf(buf, "%s: impossible to open \'%s/%s\' in reading: Permission denied\n", CMD_NAME, src_tar, src_file);
    write(STDERR_FILENO, buf, strlen(buf));
    return -1;
  }
  return 0;
}

static int has_rights_dest(char *dest_tar, char *dest_file)
{
  if(is_empty_string(dest_file))return 0;
  if(tar_access(dest_tar, dest_file, X_OK) < 0)
  {
    char buf[PATH_MAX+100];
    sprintf(buf, "%s: impossible to acceed to \'%s/%s\': Permission denied\n", CMD_NAME, dest_tar, dest_file);
    write(STDERR_FILENO, buf, strlen(buf));
    return -1;
  }
  if(tar_access(dest_tar, dest_file, W_OK) < 0)
  {
    char buf[PATH_MAX+100];
    sprintf(buf, "%s: impossible to create the standard file \'%s/%s\': Permission denied\n", CMD_NAME, dest_tar, dest_file);
    write(STDERR_FILENO, buf, strlen(buf));
    return -1;
  }
  return 0;
}

static int has_rights_src_ext(char *src_file)
{
  if(access(src_file, R_OK) < 0)
  {
    char buf[PATH_MAX+100];
    sprintf(buf, "%s: impossible to open \'%s\' in reading: Permission denied\n", CMD_NAME, src_file);
    write(STDERR_FILENO, buf, strlen(buf));
    return -1;
  }
  return 0;
}

static int has_rights_dest_ext(char *dest_file)
{
  if(access(dest_file, X_OK) < 0)
  {
    char buf[PATH_MAX+100];
    sprintf(buf, "%s: impossible to acceed to \'%s\': Permission denied\n", CMD_NAME, dest_file);
    write(STDERR_FILENO, buf, strlen(buf));
    return -1;
  }
  if(access(dest_file, W_OK) < 0)
  {
    char buf[PATH_MAX+100];
    sprintf(buf, "%s: impossible to create the standard file \'%s\': Permission denied\n", CMD_NAME, dest_file);
    write(STDERR_FILENO, buf, strlen(buf));
    return -1;
  }
  return 0;
}

static void append_slash_filename(char *filename)
{
  char *tmp = append_slash(filename);
  filename[0] = '\0';
  strcpy(filename, tmp);
  free(tmp);
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
  printf("%s/%s\n%s/%s\n\n", src_tar, src_file, dest_file, buf2);
  if(exist(dest_tar, buf2, 0) > 0)
  {
    if(tar_rm(dest_tar, buf2) < 0)
    {
      write(STDERR_FILENO, "cp: Problems on removing the file of the same name\n", 51);
      return -1;
    }
  }
  if(add_tar_to_tar(src_tar, dest_tar, src_file, buf2) < 0)
  {
    write(STDERR_FILENO, "cp: Problems at the add of file\n", 33);
    return -1;
  }
  return 0;
}

static int cp_ttt_without_r(char *src_tar, char *src_file, char *dest_tar, char *dest_file)
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
  if(has_rights_src(src_tar, src_file) < 0)
    return -1;
  if(is_dir(dest_tar, dest_file))
  {
    return (when_is_dir_dest(src_tar, src_file, dest_tar, dest_file) < 0)?-1:0;
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

static int cp_r_ttt(char *src_tar, char *src_file, char *dest_tar, char *dest_file)
{
  if(!is_dir(src_tar, src_file))
    return cp_ttt_without_r(src_tar, src_file, dest_tar, dest_file);
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

    return add_tar_to_tar_rec(src_tar, dest_tar_copy, src_file, dest_file);
  }

  int new = 0;
  if(!is_dir(dest_tar, dest_file))
  {
    if(exist(dest_tar, dest_file, 1) > 0)
    {
      char buf[PATH_MAX];
      sprintf(buf, "%s/%s", src_tar, src_file);
      char buf2[PATH_MAX];
      sprintf(buf2, "%s/%s", dest_tar, dest_file);
      return isfile_err_cp(buf, buf2);
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
  if(is_dir(src_tar, src_file))
    append_slash_filename(src_file);

  if(exist(src_tar, src_file, 1) < 0){
    char buf[PATH_MAX];
    sprintf(buf, "%s/%s", src_tar, src_file);
    return dont_exist(buf);
  }
  if(has_rights_src(src_tar, src_file) < 0){
    return -1;
  }
  if(new == 1){
    if(add_tar_to_tar_rec(src_tar, dest_tar, src_file, dest_file) < 0)
    {
      char err[33];
      sprintf(err, "%s: Problems at the add of file\n", CMD_NAME);
      write(STDERR_FILENO, err, strlen(err));
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
      strcpy(buf, src_file);

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
      free(buf);
      return -1;
    }
    if(add_tar_to_tar_rec(src_tar, dest_tar, src_file, buf2) < 0)
    {
      char err[33];
      sprintf(err, "%s: Problems at the add of file\n", CMD_NAME);
      write(STDERR_FILENO, err, strlen(err));
      free(buf);
      return -1;
    }
    free(buf);
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
    if(is_empty_string(opt))
      return already_exist(src, dest);
    else
    {
      char buf[PATH_MAX+100];
      sprintf(buf, "%s: impossible to create the directory (\'%s\') in himself (\'%s/%s\')", CMD_NAME, src_file, src_file, src_file);
      write(STDERR_FILENO, buf, strlen(buf));
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
    return isdir_err_cp(src_file);
  if(exist_ext(src_file, 1) == 0)
    return dont_exist(src_file);
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

    if(exist(dest_tar, dest_file, 1) == 0)
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
      char buf2[PATH_MAX];
      sprintf(buf2, "%s/%s", dest_tar, dest_file);
      return isfile_err_cp(src_file, buf2);
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

  if(exist_ext(src_file, 0) == 0){
    return dont_exist(src_file);
  }
  if(has_rights_src_ext(src_file) < 0){
    return -1;
  }
  if(new == 1){
    if(add_ext_to_tar_rec(dest_tar, src_file, dest_file, 0) < 0)
    {
      char err[33];
      sprintf(err, "%s: Problems at the add of file\n", CMD_NAME);
      write(STDERR_FILENO, err, strlen(err));
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
      char err[33];
      sprintf(err, "%s: Problems at the add of file\n", CMD_NAME);
      write(STDERR_FILENO, err, strlen(err));
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
    write(STDERR_FILENO, "Error fork()\n", 13);
    return -1;
  }
  if(a == -2)
  {
    write(STDERR_FILENO, "Error execlp()\n", 15);
    return -1;
  }
  return -1;
}

static int cp_tte_without_r(char *src_tar, char *src_file, char *dest_file)
{
  if(is_dir(src_tar, src_file)){
    char buf[PATH_MAX];
    sprintf(buf, "%s/%s", src_tar, src_file);
    return isdir_err_cp(buf);
  }
  if(exist(src_tar, src_file, 1) == -1){
    char buf[PATH_MAX];
    sprintf(buf, "%s/%s", src_tar, src_file);
    return dont_exist(buf);
  }
  if(has_rights_src(src_tar, src_file) < 0)
    return -1;


  if(exist_ext(dest_file, 1)==0){
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
      error_cmd(CMD_NAME, buf2);
      return -1;
    }
    if(tar_cp_file(src_tar, src_file, fd) < 0)
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
      error_cmd(CMD_NAME, dest_file);
      return -1;
    }
    if(tar_cp_file(src_tar, src_file, fd) < 0)
    {
      char err[33];
      sprintf(err, "%s: Problems at the add of file\n", CMD_NAME);
      write(STDERR_FILENO, err, strlen(err));
      return -1;
    }
  }
  return 0;
}

static int cp_r_tte(char *src_tar, char *src_file, char *dest_file)
{
  if(!is_dir(src_tar, src_file))return cp_tte_without_r(src_tar, src_file, dest_file);

  int new = 0;
  if(!is_dir_ext(dest_file))
  {
    if(exist_ext(dest_file, 1) == 0)
    {
      char buf2[PATH_MAX];
      sprintf(buf2, "%s/%s", src_tar, src_file);
      return isfile_err_cp(buf2, dest_file);
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

  if(exist(src_tar, src_file, 0) == 0){
    char buf2[PATH_MAX];
    sprintf(buf2, "%s/%s", src_tar, src_file);
    return dont_exist(buf2);
  }
  if(has_rights_src(src_tar, src_file) < 0){
    return -1;
  }
  if(new == 1){
    int a = rm_touch(dest_file, 2);
    if(a < 0)
      return error_rm_touch(a);
    if(tar_extract(src_tar, src_file, dest_file) < 0)
    {
      char err[33];
      sprintf(err, "%s: Problems at the add of file\n", CMD_NAME);
      write(STDERR_FILENO, err, strlen(err));
      return -1;
    }
  }
  else
  {
    if(tar_extract(src_tar, src_file, dest_file) < 0)
    {
      char err[33];
      sprintf(err, "%s: Problems at the add of file\n", CMD_NAME);
      write(STDERR_FILENO, err, strlen(err));
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
