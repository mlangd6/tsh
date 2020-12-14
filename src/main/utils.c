#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "tar.h"
#include "utils.h"
#include "errors.h"

#define BUFFER_SIZE 4096

/* return the umask */
mode_t getumask(void){
  mode_t mask = umask(0);
  umask(mask);
  return mask;
}

/* Read buffer by buffer of size BUFSIZE from READ_FD and write to WRITE_FD up to COUNT. */
int read_write_buf_by_buf(int read_fd, int write_fd, size_t count, size_t bufsize)
{
  char buffer[bufsize];
  int nb_of_buf = (count + bufsize - 1) / bufsize, i = 1;

  for (; i < nb_of_buf; i++)
  {
    if( read (read_fd,  buffer, bufsize) < 0 || write(write_fd, buffer, bufsize) < 0)
	    return -1;
  }

  if (i * bufsize != count)
  {
    if (read (read_fd,  buffer, count % bufsize) < 0 || write(write_fd, buffer, count % bufsize) < 0)
	     return -1;
    }

  return 0;
}


/* Copies SIZE bytes from file descriptor FD starting at WHENCE offset to WHERE offset. */
int fmemmove(int fd, off_t whence, size_t size, off_t where)
{
  char buffer[BUFFER_SIZE];
  ssize_t read_size;
  ssize_t total_read = 0;
  ssize_t end;
  while((end = size - total_read) > 0) {
    lseek(fd, whence + total_read, SEEK_SET);
    ssize_t count = (BUFFER_SIZE > end) ? end : BUFFER_SIZE;
    if( (read_size = read(fd, buffer, count)) < 0 ) {
      return -1;
    }

    lseek(fd, where + total_read, SEEK_SET);
    if( write(fd, buffer, read_size) < 0) {
      return -1;
    }
    total_read += read_size;
  }
  lseek(fd, where, SEEK_SET);

  return 0;
}


int is_dir_name(const char *filename)
{
  int pos_last_char = strlen(filename)-1;
  return filename[pos_last_char] == '/' ? 1 : 0;
}

char *append_slash(const char *str)
{
  int length = strlen(str);
  char *copy = malloc(length+2);
  strcpy(copy, str);
  if(copy[length - 1] != '/'){
    copy[length] = '/';
    copy[length + 1] = '\0';
  }
  return copy;
}

int is_empty_string(char *filename)
{
  return ((!*filename)?1:0);
}


int is_prefix (const char *prefix, const char *str)
{
  size_t prefix_len = strlen(prefix);

  if (!strncmp(prefix, str, prefix_len))
    {
      return strlen(str) == prefix_len ? 2 : 1;
    }

  return 0;
}
