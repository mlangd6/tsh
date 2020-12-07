#include <stdio.h>
#include <string.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "redirection.h"
#include "tsh.h"
#include "path_lib.h"
#include "stack.h"

struct reset_redir {
  int fd;
  int reset_fd;
};

static void redir_append(char *s, int fd);
static void tar_redir_append(char *tar_name, char *in_tar, int fd);
static void stdout_redir(char *s);
static void stderr_redir(char *s);
static void stdout_append(char *s);
static void stderr_append(char *s);
static void stdin_redir(char *s);

stack_t *reset_fds;

static void (*redirs[])(char *) = {
  stdout_redir,
  stderr_redir,
  stdout_append,
  stderr_append,
  stdin_redir
};

void init_redirections()
{
  reset_fds = stack_create();
}

void reset_redirs()
{
  while(!stack_is_empty(reset_fds))
  {
    struct reset_redir *reset = stack_pop(reset_fds);
    dup2(reset -> reset_fd, reset -> fd);
    close(reset -> reset_fd);
  }
}

void launch_redir(redir_type r, char *arg)
{
  redirs[r](arg);
}

/* DEFINITION OF REDIRECTIONS FUNCTIONS BELOW */

static void redir_append(char *s, int fd)
{
  char path[PATH_MAX];
  if (*s == '/')
    strcpy(path, s);
  else
    sprintf(path, "%s/%s", getenv("PWD"), s);
  char resolved[PATH_MAX];
  if (! reduce_abs_path(path, resolved))
    ; // TODO: erreur
  char *in_tar;
  if ((in_tar = split_tar_abs_path(resolved)))
  {
    tar_redir_append(resolved, in_tar, fd);
  }
  else
  {
    struct reset_redir *reset = malloc(sizeof(struct reset_redir));
    reset -> reset_fd = dup(fd);
    reset -> fd = fd;
    int new_fd = open(s, O_CREAT | O_APPEND | O_WRONLY, 0666);
    dup2(new_fd, fd);
    close(new_fd);
    stack_push(reset_fds, reset);
  }


}

static void tar_redir_append(char *tar_name, char *in_tar, int fd)
{

}

static void stdout_redir(char *s)
{

}

static void stderr_redir(char *s)
{

}

static void stdout_append(char *s)
{
  redir_append(s, STDOUT_FILENO);
}

static void stderr_append(char *s)
{
  redir_append(s, STDERR_FILENO);
}

static void stdin_redir(char *s)
{

}
