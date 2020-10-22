#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "tar.h"


/* Code for set_checksum(...) and check_checksum(...) are taken from :
   https://gaufre.informatique.univ-paris-diderot.fr/klimann/systL3_2020-2021/blob/master/TP/TP1/tar.h */



/* Compute and write the checksum of a header, by adding all (unsigned) bytes in
   it (while hd->chksum is initially all ' '). Then hd->chksum is set to contain
   the octal encoding of this sum (on 6 bytes), followed by '\0' and ' '. */
void set_checksum(struct posix_header *hd)
{
  memset(hd->chksum, ' ', 8);
  unsigned int sum = 0;
  char *p = (char *)hd;
  for (int i = 0; i < BLOCKSIZE; i++) {
    sum += p[i];
  }
  sprintf(hd->chksum, "%06o", sum);
}



/* Check that the checksum of a header is correct */
int check_checksum(struct posix_header *hd) {
  unsigned int checksum = 0;
  sscanf(hd->chksum, "%o ", &checksum);
  unsigned int sum = 0;
  char *p = (char *)hd;
  for (int i = 0; i < BLOCKSIZE; i++) {
    sum += p[i];
  }
  for (int i = 0; i < 8; i++) {
    sum += ' ' - hd->chksum[i];
  }
  return (checksum == sum);
}



/* Check if the file at PATHNAME is a valid tarball.
   Return :
   1  if all header are correct
   0 if at least one header is invalid or can't read a full block
   -1 otherwise */
int is_tar(const char *tar_name)
{
  int tar_fd = open(tar_name, O_RDONLY);

  if (tar_fd < 0)
    return -1;

  struct posix_header file_header;
  int fail = 0, read_size;

  while( !fail )
    {
    if((read_size=read(tar_fd, &file_header, BLOCKSIZE)) < 0)
      return -1;
    
    if( read_size != BLOCKSIZE )
      fail = 1;
    else if (file_header.name[0] == '\0')
      break;
    else if( !check_checksum(&file_header) )
      fail = 1;
    else
      skip_file_content(tar_fd, &file_header);
  }

  close(tar_fd);
  return !fail;
}



/* if FILENAME is in the tar then return 1 and set header accordingly.
   Else return 0. If there is any kind of error return -1. IN ANY CASE THE CURSOR OF TAR_FD IS MOVED. */
int find_header(int tar_fd, const char *filename, struct posix_header *header)
{
  while (1)
    {
      if( read(tar_fd, header, BLOCKSIZE) < 0)
	{
	  return -1;
	}
      else if (header->name[0] == '\0')
	{
	  return 0;
	}
      else if (strcmp(filename, header->name) == 0)
	{
	  return 1;
	}
      else
	{
	  skip_file_content(tar_fd, header);
	}
    }

  return -1;
}


unsigned int number_of_block(unsigned int filesize)
{
  return (filesize + BLOCKSIZE - 1) >> BLOCKBITS;
}

unsigned int get_file_size(struct posix_header *hd)
{
  unsigned int file_size = 0;
  sscanf(hd->size, "%o", &file_size);
  return file_size;
}

/* If it succeed returns the number of bytes from the beginning of the file. Otherwise, -1 */
int skip_file_content(int tar_fd, struct posix_header *hd)
{
  size_t file_size = get_file_size(hd);
  return lseek(tar_fd, number_of_block(file_size) * BLOCKSIZE, SEEK_CUR);
}
