#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "errors.h"

static void close_fds (int fds[], int length_fds)
{
  for (int i = 0; i < length_fds; i++)
  {
    close(fds[i]);
  }
}

static void change_errno (int new_errno)
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

void error_cmd (const char *cmd_name, const char *msg)
{
  size_t msg_len = strlen(msg);
  size_t cmd_len = strlen(cmd_name);

  char buf[cmd_len + 3 + msg_len]; // 3 = ": " + '\0'

  strcpy(buf, cmd_name);
  strcpy(buf + cmd_len, ": ");
  strcpy(buf + cmd_len + 2, msg);
  
  perror(buf);
}

void tar_error_cmd (const char *cmd_name, const char *tar_name, const char *filename)
{
  size_t cmd_len = strlen(cmd_name);
  size_t tar_name_len = strlen(tar_name);
  size_t filename_len = strlen(filename);
  size_t buf_len = cmd_len + tar_name_len + filename_len + 4; // 4 = ": " + '/' + '\0'

  char buf[buf_len];
  char *dst = buf;
  
  strcpy(dst, cmd_name);
  dst += cmd_len;
  
  strcpy(dst, ": ");
  dst += 2;
  
  strcpy(dst, tar_name);
  dst += tar_name_len;

  if (dst[-1] != '/')
    {
      strcpy(dst, "/");
      dst++;      
    }

  strcpy(dst, filename);
  
  perror(buf);
}
