#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "errors.h"
#include "tar.h"

static int seek_end_of_tar(int tar_fd) {
  struct posix_header hd;
  ssize_t read_size;
  while(1) {
    read_size = read(tar_fd, &hd, BLOCKSIZE);

    if(read_size != BLOCKSIZE)
      return -1;

    if (hd.name[0] != '\0')
      skip_file_content(tar_fd, &hd);
    else
      break;
  }

  lseek(tar_fd, -BLOCKSIZE, SEEK_CUR);
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
  hd -> mode[4] = '0' + u_rights;
  hd -> mode[5] = '0' + g_rights;
  hd -> mode[6] = '0' + o_rights;
  hd -> mode[7] = '\0';
}

static int init_header(struct posix_header *hd, const char *source, const char *filename) {
  struct stat s;
  time_t act_time;
  time(&act_time);
  if (stat(source, &s) < 0) {
    return -1;
  }
  strcpy(hd -> name, filename);
  init_mode(hd, &s);
  sprintf(hd -> uid, "%07o", s.st_uid);
  sprintf(hd -> gid, "%07o" ,s.st_gid);
  sprintf(hd -> size, "%011lo", s.st_size);
  sprintf(hd -> mtime, "%011o", (unsigned int) act_time);
  init_type(hd, &s);
  strcpy(hd -> magic, TMAGIC);
  hd -> version[0] = '0';
  hd -> version[1] = '0';
  // uname and gname are not added yet !
  set_checksum(hd);
  return 0;
}

static int init_header_empty_file(struct posix_header *hd, const char *filename){
  time_t act_time;
  time(&act_time);

  strcpy(hd -> name, filename);
  strcpy(hd -> mode, "0000644");
  sprintf(hd -> uid, "%07o", getuid());
  sprintf(hd -> gid, "%07o", getgid());
  strcpy(hd -> size, "0");
  sprintf(hd -> mtime, "%011o", (unsigned int) act_time);
  strcpy(hd -> typeflag, REGTYPE);
  strcpy(hd -> magic, TMAGIC);
  hd -> version[0] = '0';
  hd -> version[1] = '0';
  //uname and gname are not added yet
  //devmajor devminor prefix junk
  set_checksum(hd);
  return 0;
}

static int init_header_new_repertory(struct posix_header *hd, const char *filename){
  time_t act_time;
  time(&act_time);

  strcpy(hd -> name, filename);
  strcpy(hd -> mode, "0000755");
  sprintf(hd -> uid, "%07o", getuid());
  sprintf(hd -> gid, "%07o", getgid());
  strcpy(hd -> size, "0");
  sprintf(hd -> mtime, "%011o", (unsigned int) act_time);
  strcpy(hd -> typeflag, DIRTYPE);
  strcpy(hd -> magic, TMAGIC);
  hd -> version[0] = '0';
  hd -> version[1] = '0';
  //uname and gname are not added yet
  //devmajor devminor prefix junk
  set_checksum(hd);
  return 0;
}

/* Add two empty blocks at the end of a tar file */
static int add_empty_block(int tar_fd) {
  int size = 2 * BLOCKSIZE;
  char buf[size];
  memset(buf, '\0', size);
  if (write(tar_fd, buf, size) < 0) {
    return -1;
  }
  return 0;
}

/* Add file at path FILENAME to tar at path TAR_NAME */
int tar_add_file(const char *tar_name, const char *source, const char *filename) {
  int src_fd = -1;
  if ((src_fd = open(source, O_RDONLY)) < 0) {
    return error_pt(source, &src_fd, 1);
  }
  int tar_fd = open(tar_name, O_RDWR);
  int fds[2] = {src_fd, tar_fd};
  if ( tar_fd < 0) {
    return error_pt(tar_name, fds, 2);
  }
  struct posix_header hd;
  memset(&hd, '\0', BLOCKSIZE);
  if (seek_end_of_tar(tar_fd) < 0) {
    return error_pt(tar_name, fds, 2);
  }
  if(source != NULL){
    if(init_header(&hd, source, filename) < 0) {
      return error_pt(filename, fds, 2);
    }
  }
  else if(source == NULL){
    if(filename[strlen(filename)-1] == '/'){
      init_header_empty_file(&hd, filename);
    }
    else{
      init_header_new_repertory(&hd, filename);
    }
  }
  if (write(tar_fd, &hd, BLOCKSIZE) < 0) {
    return error_pt(tar_name, fds, 2);
  }

  char buffer[BLOCKSIZE];
  ssize_t read_size;
  while((read_size = read(src_fd, buffer, BLOCKSIZE)) > 0) {
    if (read_size < BLOCKSIZE) {
      memset(buffer + read_size, '\0', BLOCKSIZE - read_size);
    }
    if (write(tar_fd, buffer, BLOCKSIZE) < 0) {
      return error_pt(tar_name, fds, 2);
    }
  }
  if (read_size < 0) {
    int fds[2] ={src_fd, tar_fd};
    return error_pt(filename, fds, 2);
  }
  add_empty_block(tar_fd);
  return 0;
}
