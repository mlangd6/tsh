#include "tar.h"
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFSIZE 512

/* Compute and write the checksum of a header, by adding all (unsigned) bytes in
   it (while hd->chksum is initially all ' '). Then hd->chksum is set to contain
   the octal encoding of this sum (on 6 bytes), followed by '\0' and ' '. Code
   recupered for set_checksum(...) and check_checksum(...) on
   https://gaufre.informatique.univ-paris-diderot.fr/klimann/systL3_2020-2021/blob/master/TP/TP1/tar.h
*/

void set_checksum(struct posix_header *hd) {
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
  unsigned int checksum;
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

static int number_of_block(unsigned int filesize) {
  return (filesize + BLOCKSIZE - 1) >> BLOCKBITS;
}


/* count the number of file in a file .tar */
static int nb_file_in_tar(int fd)
{
  int i = 0;
  int n;
  struct posix_header header;

  while ( (n = read(fd, &header, BLOCKSIZE)) > 0 )
  {
    if (strcmp(header.name, "\0") == 0) break;
    else i++;
    int taille = 0;
    sscanf(header.size, "%o", &taille);
    int filesize = ((taille + BLOCKSIZE - 1) / BLOCKSIZE);
    lseek(fd, BLOCKSIZE*filesize, SEEK_CUR);
  }

  lseek(fd, 0, SEEK_SET);
  return i;
}

char **tar_ls(char *tar_name)
{
  int fd = open(tar_name, O_RDONLY);
  if (fd == -1)
  {
    perror(tar_name);
    close(fd);
    return NULL;
  }
  int n;
  int i = 0;
  struct posix_header header;
  char **ls = malloc( nb_file_in_tar(fd) * sizeof(char *) );
  assert(ls);

  while ( (n = read(fd, &header, BLOCKSIZE)) > 0 )
  {
    if (strcmp(header.name, "\0") == 0) break;
    ls[i] = malloc(strlen(header.name) + 1);
    assert(ls[i]);
    strcpy(ls[i++], header.name);
    int taille = 0;
    sscanf(header.size, "%o", &taille);
    int filesize = ((taille + BLOCKSIZE - 1) / BLOCKSIZE);
    lseek(fd, BLOCKSIZE*filesize, SEEK_CUR);
  }
  close(fd);
  return ls;
}
/* Read buffer by buffer of size BUFSIZE from READ_FD and write to WRITE_FD up to COUNT. */
static int read_write_buf_by_buf(int read_fd, int write_fd, size_t count) {
  char buffer[BUFSIZE];
  int nb_of_buf = (count + BUFSIZE - 1) / BUFSIZE, i = 1;

  for (; i < nb_of_buf; i++) {
    if( read (read_fd,  buffer, BUFSIZE) < 0 ||
	write(write_fd, buffer, BUFSIZE) < 0)
      return -1;
  }

  if (i * BUFSIZE != count) {
    if (read (read_fd,  buffer, count % BUFSIZE) < 0 ||
	write(write_fd, buffer, count % BUFSIZE) < 0)
      return -1;
  }

  return 0;
}

/* Open the tarball TAR_NAME and copy the content of FILENAME into FD.
   If FILENAME is not in the tarball or there are errors return -1, otherwise return 0. */
int tar_read_file(const char *tar_name, const char *filename, int fd) {
  int tar_fd = open(tar_name, O_RDONLY);

  if (tar_fd < 0)
    goto error_tar;

  unsigned int file_size;
  struct posix_header file_header;
  int found = 0;

  while ( !found ) {
    if( read(tar_fd, &file_header, BLOCKSIZE) < 0)
      goto error_tar;

    /* On trouve le bon nom i.e. le bon fichier */
    if (strcmp(filename, file_header.name) == 0)
    {
      /* On vérifie qu'il s'agit bien d'un fichier */
      if (file_header.typeflag == AREGTYPE || file_header.typeflag == REGTYPE){
        found = 1;
        sscanf(file_header.size, "%o", &file_size);
	if( read_write_buf_by_buf(tar_fd, fd, file_size) < 0)
	  goto error_tar;
      } else
        found = -1;

      /* On atteint les blocs nuls de fin */
    } else if (file_header.name[0] == '\0') {
      found = -1;
    } else {
      /* On saute le contenu du fichier */
      sscanf(file_header.size, "%o", &file_size);
      lseek(tar_fd, number_of_block(file_size) * BLOCKSIZE, SEEK_CUR);
    }
  }

  return found == 1 ? 0 : -1;

 error_tar:
  perror(tar_name);
  if( tar_fd != -1 )
    close(tar_fd);
  return -1;

}
