#include "errors.h"
#include <unistd.h>
#include <stdio.h>

void *error_p(const char *s, int fds[], int length_fds)
{
  perror(s);
  for(int i = 0; i < length_fds; i++)
  {
    close(fds[i]);
  }
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
