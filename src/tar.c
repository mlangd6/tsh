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
  if (header.name[0] != '\0') {
    lseek(tar_fd, BLOCKSIZE, SEEK_CUR);
    return 0;
  }
  return 0;
}

static void init_type(struct posix_header *hd, struct stat *s) {
  if (S_ISREG(s -> st_mode)) {
    hd -> typeflag = REGTYPE;
  } else if (S_ISDIR(s -> st_mode)) {
    hd -> typeflag = DIRTYPE;
  } else if (S_ISCHR(s -> st_mode)) {
    hd -> typeflag = CHRTYPE;
  } else if (S_ISBLK(s -> st_mode)) {
    hd -> typeflag = BLKTYPE;
  } else if (S_ISLNK(s -> st_mode)) {
    hd -> typeflag = LNKTYPE;
  } else if (S_ISFIFO(s -> st_mode)) {
    hd -> typeflag = FIFOTYPE;
  } else {
    hd -> typeflag = AREGTYPE;
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

static int init_header(struct posix_header *hd, const char *filename) {
  struct stat s;
  if (stat(filename, &s) < 0) {
    perror(filename);
    return -1;
  }
  strcpy(hd -> name, filename);
  init_mode(hd, &s);
  sprintf(hd -> uid, "%o", s.st_uid);
  sprintf(hd -> gid, "%o" ,s.st_gid);
  sprintf(hd -> size, "%lo", s.st_size);
  sprintf(hd -> mtime, "%lo", s.st_mtim.tv_sec);
  init_type(hd, &s);
  strcpy(hd -> magic, TMAGIC);
  strcpy(hd -> version, TVERSION);
  // uname and gname are not added yet !
  set_checksum(hd);
  return 0;

}

/* Add two empty block at the end of a tar file */
static int add_empty_block(int tar_fd) {
  int size = 2 * BLOCKSIZE;
  char buf[size];
  memset(buf, '\0', size);
  if (write(tar_fd, buf, size) < 0) {
    return -1;
  }
  return 0;
}


int tar_add_file(const char *tar_name, const char *filename) {
  int src_fd = -1;
  if ((src_fd = open(filename, O_RDONLY)) < 0) {
    goto error_file;
  }
  int tar_fd = -1;
  if ((tar_fd = open(tar_name, O_WRONLY & O_RDONLY)) < 0) {
    goto error_tar;
  }
  struct posix_header hd;
  if (seek_end_of_tar(tar_fd, tar_name) < 0) {
    goto error_tar;
  }
  init_header(&hd, filename);

  char buffer[BLOCKSIZE];
  ssize_t read_size;
  while((read_size = read(src_fd, buffer, BLOCKSIZE)) > 0) {
    if (read_size < BLOCKSIZE) {
      memset(buffer + read_size, '\0', BLOCKSIZE - read_size);
    }
    if (write(tar_fd, buffer, BLOCKSIZE) < 0) {
      goto error_tar;
    }
  }
  if (read_size < 0) {
    goto error_file;
  }
  add_empty_block(tar_fd);
  return 0;
error_file:
  perror(filename);
  if (src_fd != -1) {
    close(src_fd);
  }
  if (tar_fd != -1) {
    close(tar_fd);
  }
  return -1;

error_tar:
  perror(tar_name);
  if (tar_fd != -1) {
    close(tar_fd);
  }
  if (src_fd != -1) {
    close(src_fd);
  }
  return -1;
}
