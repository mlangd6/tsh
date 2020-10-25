#include "errors.h"
#include <stdlib.h>
#include <string.h>
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

int write_error(const char *pre_error, const char *arg, const char *post_error) {
  char *error = malloc(strlen(pre_error) + strlen(arg) + strlen(post_error) + 1);
  memcpy(error, pre_error, strlen(pre_error));
  memcpy(error, arg, strlen(arg));
  memcpy(error, post_error, strlen(post_error) + 1);
  write(STDERR_FILENO, error, strlen(error) + 1);
  free(error);
  return EXIT_FAILURE;
}
