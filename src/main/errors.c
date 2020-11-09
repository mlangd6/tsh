#include "errors.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

static void close_fds(int fds[], int length_fds)
{
  for (int i = 0; i < length_fds; i++)
  {
    close(fds[i]);
  }
}

static void change_errno(int new_errno)
{
  if (0 < new_errno)
  {
    errno = new_errno;
  }
}

void *error_p(int fds[], int length_fds, int new_errno)
{
  close_fds(fds, length_fds);
  change_errno(new_errno);
  return NULL;
}

int error_pt(int fds[], int length_fds, int new_errno)
{
  close_fds(fds, length_fds);
  change_errno(new_errno);
  return -1;
}

void error_cmd(const char *cmd_name, const char *msg) {
  int msg_len = strlen(msg);
  int cmd_len = strlen(cmd_name);
  char buf[cmd_len + 3 + msg_len];
  memcpy(buf, cmd_name, cmd_len);
  memcpy(buf + cmd_len, ": ", 2);
  memcpy(buf + cmd_len + 2, msg, msg_len+1);
  perror(buf);
}
