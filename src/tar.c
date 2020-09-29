#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "tar.h"

/* Compute and write the checksum of a header, by adding all (unsigned) bytes in it (while hd->chksum is initially all ' '). Then hd->chksum is set to contain the octal encoding of this sum (on 6 bytes), followed by '\0' and ' '.
   Code recupered for set_checksum(...) and check_checksum(...) on https://gaufre.informatique.univ-paris-diderot.fr/klimann/systL3_2020-2021/blob/master/TP/TP1/tar.h
*/

void set_checksum(struct posix_header *hd) {
  memset(hd->chksum,' ',8);
  unsigned int sum = 0;
  char *p = (char *)hd;
  for (int i=0; i < BLOCKSIZE; i++) { sum += p[i]; }
  sprintf(hd->chksum,"%06o",sum);
}

/* Check that the checksum of a header is correct */

int check_checksum(struct posix_header *hd) {
  unsigned int checksum;
  sscanf(hd->chksum,"%o ", &checksum);
  unsigned int sum = 0;
  char *p = (char *)hd;
  for (int i=0; i < BLOCKSIZE; i++) { sum += p[i]; }
  for (int i=0;i<8;i++) { sum += ' ' - hd->chksum[i]; }
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
  struct posix_header *header = malloc(sizeof(struct posix_header));
  assert(header);

  while ( (n = read(fd, header, BLOCKSIZE)) > 0 )
  {
    if (strcmp(header->name, "\0") == 0) break;
    else i++;
    int taille = 0;
    sscanf(header->size, "%o", &taille);
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
    return NULL;
  }
  int n;
  int i = 0;
  struct posix_header *header = malloc(sizeof(struct posix_header));
  assert(header);
  char **ls = malloc( nb_file_in_tar(fd) * sizeof(char *) );
  assert(ls);

  while ( (n = read(fd, header, BLOCKSIZE)) > 0 )
  {
    if (strcmp(header->name, "\0") == 0) break;
    ls[i] = malloc(strlen(header->name) + 1);
    assert(ls[i]);
    strcpy(ls[i++], header->name);
    int taille = 0;
    sscanf(header->size, "%o", &taille);
    int filesize = ((taille + BLOCKSIZE - 1) / BLOCKSIZE);
    lseek(fd, BLOCKSIZE*filesize, SEEK_CUR);
  }
  close(fd);
  return ls;
}
