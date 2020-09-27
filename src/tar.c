#include "tar.h"
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

/* Read buffer by buffer of size BUFSIZE from READ_FD and write to WRITE_FD up to COUNT. */
static void read_write_buf_by_buf(int read_fd, int write_fd, size_t count) {
  char buffer[BUFSIZE];
  int nb_of_buf = (count + BUFSIZE - 1) / BUFSIZE, i = 1;
  
  for (; i < nb_of_buf; i++) {
    read (read_fd,  buffer, BUFSIZE);
    write(write_fd, buffer, BUFSIZE);
  }
  
  if (i * BUFSIZE != count) {
    read (read_fd,  buffer, count % BUFSIZE);
    write(write_fd, buffer, count % BUFSIZE);
  }  
}

/* Open the tarball TAR_NAME and copy the content of FILENAME into FD.
   If FILENAME is not in the tarball or there are errors return -1, otherwise return 0. */
// TODO: Il n'y a pas de gestion des erreurs, trouver une manière élégante de le faire !
int tar_read_file(const char *tar_name, const char *filename, int fd) {
  int tar_fd = open(tar_name, O_RDONLY);

  if (tar_fd < 0)
    return -1;

  unsigned int file_size;
  struct posix_header file_header;
  int found = 0;
    
  while ( !found ) {
    read(tar_fd, &file_header, BLOCKSIZE);

    /* On trouve le bon nom i.e. le bon fichier */
    if (strcmp(filename, file_header.name) == 0)
    {
      /* On vérifie qu'il ne s'agit pas d'un dossier */
      if (file_header.typeflag == '5') // TODO: il existe une macro dans le fichier tar.h POSIX 1003.1-1990
        found = -1;
      else {
        found = 1;
        sscanf(file_header.size, "%o", &file_size);
	read_write_buf_by_buf(tar_fd, fd, file_size);
      }
      /* On atteint les blocs nuls de fin */
    } else if (file_header.name[0] == '\0') {
      found = -1;
    } else {
      /* On saute le contenu du fichier */
      sscanf(file_header.size, "%o", &file_size);
      lseek(tar_fd, number_of_block(file_size) * BLOCKSIZE, SEEK_CUR);
    }
  }

  close(tar_fd);

  return found == 1 ? 0 : -1;
}
