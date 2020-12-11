#include <stdio.h>
#include <string.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

#include "redirection.h"
#include "tsh.h"
#include "path_lib.h"
#include "stack.h"
#include "tar.h"
#include "utils.h"
#include "errors.h"

struct reset_redir {
  int fd;
  int reset_fd;
};

static int redir_append(char *s, int fd);
static int tar_redir_append(char *tar_name, char *in_tar, int fd);
static int stdout_redir(char *s);
static int stderr_redir(char *s);
static int stdout_append(char *s);
static int stderr_append(char *s);
static int stdin_redir(char *s);
static void add_reset_redir(int fd);
static int handle_inside_tar_redir(int fd, char *tar_name, char *in_tar);

stack_t *reset_fds;

static int (*redirs[])(char *) = {
  stdout_redir,
  stderr_redir,
  stdout_append,
  stderr_append,
  stdin_redir
};

/* Add a reset struct to the stack of the reseter of redirections */
static void add_reset_redir(int fd)
{
  struct reset_redir *reset = malloc(sizeof(struct reset_redir));
  reset -> reset_fd = dup(fd);
  reset -> fd = fd;
  stack_push(reset_fds, reset);
}

/* Init the stack of reset struct */
void init_redirections()
{
  reset_fds = stack_create();
}

void exit_redirections()
{
  reset_redirs(); // In case there is redirections while exiting
  stack_free(reset_fds, true);
}

/* Reset all redirections that has been launched */
void reset_redirs()
{
  while(!stack_is_empty(reset_fds))
  {
    struct reset_redir *reset = stack_pop(reset_fds);
    dup2(reset -> reset_fd, reset -> fd);
    close(reset -> reset_fd);
    free(reset);
  }
}

/* Launch redirection of type r to file name arg */
int launch_redir(redir_type r, char *arg)
{
  return redirs[r](arg);
}

/* DEFINITION OF REDIRECTIONS FUNCTIONS BELOW */

/* Function to handle both append redirections */
static int redir_append(char *s, int fd)
{
  char path[PATH_MAX];
  if (*s == '/')
    strcpy(path, s);
  else
    sprintf(path, "%s/%s", getenv("PWD"), s);
  char resolved[PATH_MAX];
  if (! reduce_abs_path(path, resolved))
  {
    error_cmd("tsh", path);
  }
  char *in_tar;
  if ((in_tar = split_tar_abs_path(resolved)))
  {
    return tar_redir_append(resolved, in_tar, fd);
  }
  else
  {
    add_reset_redir(fd);
    int new_fd = open(s, O_CREAT | O_APPEND | O_WRONLY, 0666);
    if (new_fd < 0)
    {
      error_cmd("tsh", s);
      return -1;
    }
    dup2(new_fd, fd);
    close(new_fd);
    return 0;
  }


}

/* Handle main loop for > >> 2> 2>> redirections inside tar */
static int handle_inside_tar_redir(int fd, char *tar_name, char *in_tar)
{
  int pipefd[2];
  if (pipe(pipefd) == -1)
  {
    perror("Redirections: pipe");
    return -1;
  }
  add_reset_redir(fd);
  dup2(pipefd[1], fd);
  close(pipefd[1]);
  switch(fork())
  {
    case -1:
      perror("Redirections: fork");
      break;
    case 0: // child
    {
      int tar_fd = open(tar_name, O_RDWR);
      ssize_t read_size;
      char buff[4096];
      struct posix_header hd;
      while(1) {
        switch((read_size = read(pipefd[0], buff, 4096)))
        {
          case -1:
            perror("read on pipe");
          case 0:
            break;
          default:
          {
            void update_size(struct posix_header *hd)
            {
              int size = get_file_size(hd);
              long unsigned int new_size = size + read_size;
              sprintf(hd -> size, "%011lo", new_size);
            }
            update_header(&hd, tar_fd, in_tar, update_size);
            int old_size = get_file_size(&hd) - read_size;
            int padding = BLOCKSIZE - (old_size % BLOCKSIZE);
            off_t beg = lseek(tar_fd, old_size, SEEK_CUR);
            off_t tar_size = lseek(tar_fd, 0, SEEK_END);

            int required_blocks = read_size <= padding ? 0 : number_of_block(read_size);
            if (fmemmove(tar_fd, beg + padding, tar_size - (beg + padding), beg + padding + required_blocks*BLOCKSIZE)) {
              close(tar_fd);
              return -1;
            }
            lseek(tar_fd, beg, SEEK_SET);
            write(tar_fd, buff, read_size);
            break;
          }
        }
      }
      break;
    }
    default: // parent
      close(pipefd[0]);
      break;
  }
  return 0;
}

/* Handle >> 2>> redirections inside tar */
static int tar_redir_append(char *tar_name, char *in_tar, int fd)
{
  if (tar_access(tar_name, in_tar, W_OK) != 1)
  {
    if (errno == EACCES)
    {
      goto error;
    }
    else if (errno == ENOENT)
    {
      tar_add_file(tar_name, NULL, in_tar);
    }
  }
  if (in_tar[strlen(in_tar) - 1] == '/')
  {
    errno = EISDIR;
    goto error;
  }
  handle_inside_tar_redir(fd, tar_name, in_tar);
  return 0;
  error: {
    char error[PATH_MAX];
    sprintf(error, "%s/%s", tar_name, in_tar);
    error_cmd("tsh", error);
    return -1;
  }
}

static int stdout_redir(char *s)
{
  return 0;
}

static int stderr_redir(char *s)
{
  return 0;
}

static int stdout_append(char *s)
{
  return redir_append(s, STDOUT_FILENO);
}

static int stderr_append(char *s)
{
  return redir_append(s, STDERR_FILENO);
}

static int stdin_redir(char *s)
{
  return 0;
}
