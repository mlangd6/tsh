#include "errors.h"
#include <unistd.h>
#include <stdio.h>

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
