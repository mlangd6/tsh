#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "errors.h"
#include "tar.h"




/* List all files contained in the tar at path TAR_NAME */
struct posix_header *tar_ls(const char *tar_name)
{
  int tar_fd = open(tar_name, O_RDONLY);
  if (tar_fd < 0)
    return error_p(&tar_fd, 1);

  int nb_file = nb_files_in_tar(tar_fd);
  if(nb_file < 0)
    return error_p(&tar_fd, 1);

  ssize_t size_read;

  struct posix_header *list_header = malloc(nb_file * sizeof(struct posix_header));
  assert(list_header);

  for(int i=0; i < nb_file; i++)
  {
    size_read = read(tar_fd, list_header+i, BLOCKSIZE);

    if(size_read < 0) {
      free(list_header);
      return error_p(&tar_fd, 1);
    }
    else if (size_read != BLOCKSIZE) {
      free(list_header);
      errno = EINVAL;
      return error_p(&tar_fd, 1);
    }

    skip_file_content(tar_fd, list_header+i);
  }

  close(tar_fd);
  return list_header;
}
