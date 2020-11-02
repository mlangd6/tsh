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

/* Open the tar at path TAR_NAME and copy the content of FILENAME into FD then delete FILENAME from the tar */
int tar_mv_file(const char *tar_name, const char *filename, int fd)
{
  int tar_fd = open(tar_name, O_RDWR);

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
  } else if (file_header.typeflag != AREGTYPE && file_header.typeflag != REGTYPE) { // pas un fichier ou pas trouvé
    errno = EPERM;
    close(tar_fd);
    return -1;
  }

  int p = lseek(tar_fd, 0, SEEK_CUR);

  // CP
  file_size = get_file_size(&file_header);
  if( read_write_buf_by_buf(tar_fd, fd, file_size, BUFSIZE) < 0)
    return error_pt(&tar_fd, 1);

  // RM
  off_t file_start = p - BLOCKSIZE,
        file_end   = file_start + BLOCKSIZE + number_of_block(file_size)*BLOCKSIZE,
        tar_end    = lseek(tar_fd, 0, SEEK_END);

  if( fmemmove(tar_fd, file_end, tar_end - file_end, file_start) < 0)
    return error_pt(&tar_fd, 1);

  ftruncate(tar_fd, tar_end - (file_end - file_start));

  close(tar_fd);

  return 0;
}
