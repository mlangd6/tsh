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
  int nb_of_buf = count / bufsize, i = 0;

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
  char *buffer = malloc(size); // TODO: On fait un gros malloc, peut-être en faire plusieurs...
  assert(buffer);

  lseek(fd, whence, SEEK_SET);
  if( read(fd, buffer, size) < 0 )
    {
      free(buffer);
      return -1;
    }

  lseek(fd, where, SEEK_SET);
  if( write(fd, buffer, size) < 0)
    {
      free(buffer);
      return -1;
    }

  free(buffer);

  lseek(fd, where, SEEK_SET);

  return 0;
}

/* Check if FILENAME ends with '/' */
int is_dir_name(const char *filename)
{
  int pos_last_char = strlen(filename)-1;
  return filename[pos_last_char] == '/' ? 1 : 0;
}


int is_empty_string(const char *filename)
{
  return ((!*filename)?1:0);
}

char *append_slash(const char *str)
{
  if(is_empty_string(str))return NULL;
  int length = strlen(str);
  char *copy = malloc(length+2);
  strcpy(copy, str);
  if(copy[length - 1] != '/'){
    copy[length] = '/';
    copy[length + 1] = '\0';
  }
  return copy;
}


void append_slash_filename(char *filename)
{
  char *tmp = append_slash(filename);
  filename[0] = '\0';
  strcpy(filename, tmp);
  free(tmp);
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


int write_string (int fd, const char *string)
{
  return write(fd, string, strlen(string) + 1);
}


char *copy_string (const char *str)
{
  char *cpy = malloc(strlen(str)+1);
  assert(cpy);

  strcpy(cpy, str);
  return cpy;
}

void remove_last_slash(char *s)
{
  int len = strlen(s);
  if (s[len-1] == '/')
  {
    s[len-1] = '\0';
  }
}
