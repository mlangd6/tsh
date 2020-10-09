#include "tar.h"
#include "errors.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#define SIZE_OF_LINE 149

static char *concat(char **all, int size)
{
  char *line = malloc(SIZE_OF_LINE);
  for(int i = 0; i < size; i++)
  {
    strcat(line, all[i]);
    strcat(line, " ");
  }
  return line;
}

static char **add_in_line(char **line, struct posix_header ph)
{
  line[0] = ph.mode;
  line[1] = ph.uid;
  line[2] = ph.gid;
  line[3] = ph.size;
  line[4] = ph.mtime;
  line[5] = ph.name;
  return line;
}

char **ls_l(const char *tar_name) {
  struct posix_header *header = tar_ls(tar_name);
  int tar_fd = open(tar_name, O_RDONLY);
  if (tar_fd == -1)
  {
    return error_p(tar_name, &tar_fd, 1);
  }
  int nb_in_tar = nb_files_in_tar(tar_fd);
  char **lines = malloc(nb_in_tar * SIZE_OF_LINE);
  assert(lines);
  close(tar_fd);

  for(int i = 0; i < nb_in_tar; i++)
  {
    char **line = malloc(6 * 100);
    assert(line);
    add_in_line(line, header[i]);
    lines[i] = concat(line, 6);
    printf("%s\n", lines[i]);
  }
  return lines;
}

char **ls(const char *tar_name) {
  return NULL;
}
