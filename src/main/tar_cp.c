#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "errors.h"
#include "tar.h"
#include "utils.h"

#define BUFSIZE BLOCKSIZE

extern int errno;

/* Open the tarball TAR_NAME and copy the content of FILENAME into FD */
int tar_cp_file(const char *tar_name, const char *filename, int fd) {
  int tar_fd = open(tar_name, O_RDONLY);

  if (tar_fd < 0)
    return error_pt(&tar_fd, 1);

  unsigned int file_size;
  struct posix_header file_header;
  int r = seek_header(tar_fd, filename, &file_header);

  if(r < 0) // erreur
    return error_pt(&tar_fd, 1);
  else if( r == 0) {
    errno = ENOENT;
    close(tar_fd);
    return -1;
  }
  else if (file_header.typeflag == DIRTYPE) {
    errno = EISDIR;
    close(tar_fd);
    return -1;
  }
  else if (file_header.typeflag != AREGTYPE && file_header.typeflag != REGTYPE) {
      errno = EPERM;
      close(tar_fd);
      return -1;
    }

  file_size = get_file_size(&file_header);
  if( read_write_buf_by_buf(tar_fd, fd, file_size, BUFSIZE) < 0)
    return error_pt(&tar_fd, 1);

  close(tar_fd);

  return 0;
}
