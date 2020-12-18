#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>


#include "path_lib.h"
#include "command_handler.h"
#include "errors.h"
#include "tar.h"
#include "utils.h"

#define CMD_NAME "rmdir"

static int rmdir_err(int tar_fd, char *err, int new_errno, char *to_free, array *array_to_free);
static int rm_tar(int tar_fd, char *tar_name);
static int is_empty_tar_dir(int tar_fd, char *dir_cpy, char *err);
static int rmdir_case_dir(int tar_fd, char *tar_name, char *filename, char *err);
static int parent_dir_access(int tar_fd, char *dir, char *err);
static int pwd_prefix_err(char *filename);

int rmdir_cmd(char *tar_name, char *filename, char *options)
{
  char err[PATH_MAX];
  sprintf(err, "%s/%s", tar_name, filename);
  int tar_fd = open(tar_name, O_RDWR);
  if (tar_fd < 0)
    error_cmd(CMD_NAME, err);

  if (is_empty_string(filename))
    return rm_tar(tar_fd, tar_name);

  enum file_type type = ftype_of_file(tar_fd, filename, true);
  switch (type)
  {
    case DIR:
      return rmdir_case_dir(tar_fd, tar_name, filename, err);
    case REG:
      return rmdir_err(tar_fd, err, ENOTDIR, NULL, NULL);
    default: // NONE
      return rmdir_err(tar_fd, err, errno, NULL, NULL);
  }
}

static int rmdir_err(int tar_fd, char *err, int new_errno, char *to_free, array *array_to_free)
{
  if (to_free != NULL) free(to_free);
  if (array_to_free != NULL) array_free(array_to_free, false);
  close(tar_fd);
  errno = new_errno;
  error_cmd(CMD_NAME, err);
  return -1;
}

static int rm_tar(int tar_fd, char *tar_name)
{
  if (is_pwd_prefix(tar_name, ""))
  {
    close(tar_fd);
    return pwd_prefix_err(tar_name);
  }
  if (nb_files_in_tar(tar_fd) != 0)
    return rmdir_err(tar_fd, tar_name, ENOTEMPTY, NULL, NULL);
  if (unlink(tar_name) < 0)
    return rmdir_err(tar_fd, tar_name, errno, NULL, NULL);
  close(tar_fd);
  return 0;
}

static int is_empty_tar_dir(int tar_fd, char *dir_cpy, char *err)
{

  array *sub_files_rec = tar_ls_dir(tar_fd, dir_cpy, true);

  // Vérification qu'il n'y a aucun sous fichiers (y compris via des dossiers sans header)
  if (array_size(sub_files_rec) != 0)
    return rmdir_err(tar_fd, err, ENOTEMPTY, dir_cpy, sub_files_rec);
  array_free(sub_files_rec, false);
  return 0;
}

static int rmdir_case_dir(int tar_fd, char *tar_name, char *filename, char *err)
{
  if (is_pwd_prefix(tar_name, filename) == -1)
  {
    close(tar_fd);
    return pwd_prefix_err(err);
  }
  char *dir_cpy = append_slash(filename);
  if (parent_dir_access(tar_fd, dir_cpy, err) != 0) return -1;
  if (is_empty_tar_dir(tar_fd, dir_cpy, err) != 0) return -1;
  if (tar_rm_dir(tar_fd, dir_cpy) != 0) return -1;
  free(dir_cpy);
  close(tar_fd);
  return 0;
}

static int parent_dir_access(int tar_fd, char *dir, char *err)
{
  char parent[PATH_MAX];
  strcpy(parent, dir);

  parent[strlen(parent) - 1] = '\0';
  char *last_slash = strrchr(parent, '/');

  if (!last_slash)
    return 0;
  last_slash[1] = '\0';
  if (ftar_access(tar_fd, parent, W_OK | X_OK) < 0)
  {
    return rmdir_err(tar_fd, err, errno, dir, NULL);
  }
  return 0;
}

static int pwd_prefix_err(char *filename)
{
  char msg[1024];
  sprintf(msg, "%s: Cannot remove \'%s\': is prefix of current working directory\n", CMD_NAME, filename);
  write(STDERR_FILENO, msg, strlen(msg) + 1);
  return -1;
}

int main(int argc, char *argv[]) {
  unary_command cmd = {
    CMD_NAME,
    rmdir_cmd,
    false,
    false,
    ""
  };

  return handle_unary_command(cmd, argc, argv);
}
