#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "errors.h"
#include "tar.h"


/* Count the number of file in the tar referenced by TAR_FD. */
static int nb_file_in_tar(int tar_fd)
{
  ssize_t read_size;
  int nb = 0;
  struct posix_header header;

  while ((read_size = read(tar_fd, &header, BLOCKSIZE)) > 0 )
    {
      if(read_size != BLOCKSIZE)
	return -1;
      if(header.name[0] == '\0')
	break;
      else
	nb++;
      
      skip_file_content(tar_fd, &header);
    }

  lseek(tar_fd, 0, SEEK_SET);
  return nb;
}


/* Return an array of all files header of a tar located at path TAR_NAME. */
struct posix_header *tar_ls(const char *tar_name)
{
  int tar_fd = open(tar_name, O_RDONLY);
  if (tar_fd < 0)
    return error_p(tar_name, &tar_fd, 1);

  int nb_file = nb_file_in_tar(tar_fd);
  if(nb_file < 0)
    return error_p(tar_name, &tar_fd, 1);
  
  ssize_t read_size;

  struct posix_header *list_header = malloc(nb_file * sizeof(struct posix_header));
  assert(list_header);

  for(int i=0; i < nb_file; i++)
    {
      read_size = read(tar_fd, list_header+i, BLOCKSIZE);

      if(read_size != BLOCKSIZE)
	{
	  free(list_header);
	  return error_p(tar_name, &tar_fd, 1);
	}
      
      skip_file_content(tar_fd, list_header+i);
    }
  
  close(tar_fd);
  return list_header;
}
