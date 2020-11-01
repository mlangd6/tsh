#include "errors.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void *error_p(int fds[], int length_fds)
{
  for(int i = 0; i < length_fds; i++)
  {
    close(fds[i]);
  }
  return NULL;
}

int error_pt(int fds[], int length_fds)
{
  for(int i = 0; i < length_fds; i++)
  {
    close(fds[i]);
  }
  return -1;
}

void cmd_error(const char *cmd_name, const char *msg) {
  int msg_len = strlen(msg);
  int cmd_len = strlen(cmd_name);
  char buf[cmd_len + 3 + msg_len];
  memcpy(buf, cmd_name, cmd_len);
  memcpy(buf + cmd_len, ": ", 2);
  memcpy(buf + cmd_len + 2, msg, msg_len+1);
  perror(buf);
}
