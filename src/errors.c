#include "errors.h"
#include <unistd.h>
#include <stdio.h>

int errori(const char *s, int fd)
{
  perror(s);
  close(fd);
  return -1;
}

char **errorppc(const char *s, int fd)
{
  perror(s);
  close(fd);
  return NULL;
}

int errorpt(const char *s, int fds[], int length_fds)
{
  perror(s);
  for(int i = 0; i < length_fds; i++)
  {
    close(fds[i]);
  }
  return -1;
}
