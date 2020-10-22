#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "errors.h"
#include "tar.h"
#include "utils.h"

#define BUFSIZE BLOCKSIZE

/* Open the tarball TAR_NAME and copy the content of FILENAME into FD.
   If FILENAME is not in the tarball or there are errors return -1, otherwise return 0. */
int tar_cp_file(const char *tar_name, const char *filename, int fd) {
  int tar_fd = open(tar_name, O_RDONLY);

  if (tar_fd < 0)
    return error_pt(tar_name, &tar_fd, 1);

  unsigned int file_size;
  struct posix_header file_header;
  int r = find_header(tar_fd, filename, &file_header);

  if(r < 0) // erreur
    return error_pt(tar_name, &tar_fd, 1);
  else if( r == 0 || (file_header.typeflag != AREGTYPE && file_header.typeflag != REGTYPE)) // pas un fichier ou pas trouvé
    {
      close(tar_fd);
      return -1;
    }

  file_size = get_file_size(&file_header);
  if( read_write_buf_by_buf(tar_fd, fd, file_size, BUFSIZE) < 0)
    return error_pt(tar_name, &tar_fd, 1);

  close(tar_fd);

  return 0;
}
