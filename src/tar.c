#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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

static int seek_end_of_tar(int tar_fd, const char *tar_name) {
  lseek(tar_fd, -2 * BLOCKSIZE, SEEK_END);
  struct posix_header header;
  if (read(tar_fd, &header, BLOCKSIZE) < 0) {
    perror(tar_name);
    return -1;
  }
  if (header.name[0] == '\0') {
    lseek(tar_fd, -BLOCKSIZE, SEEK_CUR);
    return 0;
  }
  return 0;
}

static void init_type(struct posix_header *hd, struct stat *s) {
  switch (0) {
    case !S_ISREG(s -> st_mode) : hd -> typeflag = REGTYPE;  break;
    case !S_ISDIR(s -> st_mode) : hd -> typeflag = DIRTYPE;  break;
    case !S_ISCHR(s -> st_mode) : hd -> typeflag = CHRTYPE;  break;
    case !S_ISBLK(s -> st_mode) : hd -> typeflag = BLKTYPE;  break;
    case !S_ISLNK(s -> st_mode) : hd -> typeflag = LNKTYPE;  break;
    case !S_ISFIFO(s -> st_mode): hd -> typeflag = FIFOTYPE; break;
    default: hd -> typeflag = AREGTYPE;
  }
}

static void init_mode(struct posix_header *hd, struct stat *s) {
  strcpy(hd -> mode, "0000000");
  int u_rights = (S_IRUSR & s -> st_mode) ? 4 : 0;
  int g_rights = (S_IRGRP & s -> st_mode) ? 4 : 0;
  int o_rights = (S_IROTH & s -> st_mode) ? 4 : 0;

  u_rights += (S_IWUSR & s -> st_mode) ? 2 : 0;
  g_rights += (S_IWGRP & s -> st_mode) ? 2 : 0;
  o_rights += (S_IWOTH & s -> st_mode) ? 2 : 0;

  u_rights += (S_IXUSR & s -> st_mode) ? 1 : 0;
  g_rights += (S_IXGRP & s -> st_mode) ? 1 : 0;
  o_rights += (S_IXOTH & s -> st_mode) ? 1 : 0;

  hd -> mode[5] = '0' + g_rights;
  hd -> mode[6] = '0' + u_rights;
  hd -> mode[7] = '0' + o_rights;
}
