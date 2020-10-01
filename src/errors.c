#include "errors.h"
#include <unistd.h>
#include <stdio.h>

int error_i(const char *s, int fd)
{
  perror(s);
  close(fd);
  return -1;
}

char **error_ppc(const char *s, int fd)
{
  perror(s);
  close(fd);
  return NULL;
}

int error_pt(const char *s, int fds[], int length_fds)
{
  perror(s);
  for(int i = 0; i < length_fds; i++)
  {
    close(fds[i]);
  }
  return -1;
}
