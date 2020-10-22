#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "errors.h"
#include "tar.h"


/* count the number of file in a file .tar */
static int nb_file_in_tar(int tar_fd)
{
  int i = 0;
  int n;
  struct posix_header header;

  while ( (n = read(tar_fd, &header, BLOCKSIZE)) > 0 )
    {
      if (strcmp(header.name, "\0") == 0) break;
      else i++;
      int taille = 0;
      sscanf(header.size, "%o", &taille);
      int filesize = ((taille + BLOCKSIZE - 1) / BLOCKSIZE);
      lseek(tar_fd, BLOCKSIZE*filesize, SEEK_CUR);
    }

  lseek(tar_fd, 0, SEEK_SET);
  return i;
}


struct posix_header *tar_ls(const char *tar_name)
{
  int tar_fd = open(tar_name, O_RDONLY);
  if (tar_fd == -1)
    {
      return error_p(tar_name, &tar_fd, 1);
    }
  int n;
  int i = 0;
  struct posix_header header;
  struct posix_header *list_header = malloc(nb_file_in_tar(tar_fd)*sizeof(struct posix_header));
  assert(list_header);

  while ( (n = read(tar_fd, &header, BLOCKSIZE)) > 0 )
    {
      if (strcmp(header.name, "\0") == 0) break;
      list_header[i++] = header;
      int taille = 0;
      sscanf(header.size, "%o", &taille);
      int filesize = ((taille + BLOCKSIZE - 1) / BLOCKSIZE);
      lseek(tar_fd, BLOCKSIZE*filesize, SEEK_CUR);
    }
  close(tar_fd);
  return list_header;
}
