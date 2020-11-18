#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/limits.h>

#include "tar.h"
#include "path_lib.h"

/* Code for set_checksum(...) and check_checksum(...) are taken from :
   https://gaufre.informatique.univ-paris-diderot.fr/klimann/systL3_2020-2021/blob/master/TP/TP1/tar.h */



/* Compute and write the checksum of a header */
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


/* Check if the file at PATH is a valid tar */
int is_tar(const char *path)
{
  int tar_fd = open(path, O_RDONLY);

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


/* Seek FILENAME in the tar referenced by TAR_FD and set HEADER accordingly */
int seek_header(int tar_fd, const char *filename, struct posix_header *header)
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


/* Convert FILESIZE into a number of blocks */
unsigned int number_of_block(unsigned int filesize)
{
  return (filesize + BLOCKSIZE - 1) >> BLOCKBITS;
}


/* Return the file size from a given header */
unsigned int get_file_size(struct posix_header *hd)
{
  unsigned int file_size = 0;
  sscanf(hd->size, "%o", &file_size);
  return file_size;
}


/* Increment the file offset of TAR_FD by file size given in HD */
int skip_file_content(int tar_fd, struct posix_header *hd)
{
  size_t file_size = get_file_size(hd);
  return lseek(tar_fd, number_of_block(file_size) * BLOCKSIZE, SEEK_CUR);
}

/* Count the number of file in the tar referenced by TAR_FD */
int nb_files_in_tar(int tar_fd)
{
  ssize_t size_read;
  int nb = 0;
  struct posix_header header;

  while ((size_read = read(tar_fd, &header, BLOCKSIZE)) > 0 )
    {
      if(size_read != BLOCKSIZE)
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

int nb_files_in_tar_c(char *tar_name){
  int tar_fd = open(tar_name, O_RDONLY);
  if (tar_fd < 0){
    close(tar_fd);
    return -1;
  }
  int nb = nb_files_in_tar(tar_fd);
  close(tar_fd);
  return nb;
}
