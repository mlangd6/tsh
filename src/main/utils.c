#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"


/* Read buffer by buffer of size BUFSIZE from READ_FD and write to WRITE_FD up to COUNT. */
int read_write_buf_by_buf(int read_fd, int write_fd, size_t count, size_t bufsize)
{
  char buffer[bufsize];
  int nb_of_buf = (count + bufsize - 1) / bufsize, i = 1;

  for (; i < nb_of_buf; i++)
    {
      if( read (read_fd,  buffer, bufsize) < 0 ||
	  write(write_fd, buffer, bufsize) < 0)
	return -1;
    }

  if (i * bufsize != count)
    {
      if (read (read_fd,  buffer, count % bufsize) < 0 ||
	  write(write_fd, buffer, count % bufsize) < 0)
	return -1;
    }

  return 0;
}

int fmemmove(int tar_fd, off_t whence, size_t size, off_t where)
{
  char *buffer = malloc(size);
  assert(buffer);

  lseek(tar_fd, whence, SEEK_SET);
  if( read(tar_fd, buffer, size) < 0 )
    {
      free(buffer);
      return -1;
    }

  lseek(tar_fd, where, SEEK_SET);
  if( write(tar_fd, buffer, size) < 0)
    {
      free(buffer);
      return -1;
    }

  free(buffer);

  return 0;
}

/* Check if FILENAME ends with '/' */
int is_dir_name(const char *filename)
{
  int pos_last_char = strlen(filename)-1;
  return filename[pos_last_char] == '/' ? 1 : 0;
}

/* Check if STR starts with PREFIX */
int is_prefix(const char *prefix, const char *str)
{
  return !strncmp(prefix, str, strlen(prefix));
}
