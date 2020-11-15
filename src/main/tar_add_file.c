#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <dirent.h>
#include <linux/limits.h>

#include "errors.h"
#include "tar.h"
#include "utils.h"


static int seek_end_of_tar(int tar_fd) {
  struct posix_header hd;
  ssize_t read_size;
  while(1) {
    read_size = read(tar_fd, &hd, BLOCKSIZE);
    if (read_size != BLOCKSIZE)
    {
      if (read_size < 0)
        return -1;
      else
        return error_pt(NULL, 0, EPERM);
    }
    if (hd.name[0] != '\0')
      skip_file_content(tar_fd, &hd);
    else
      break;
  }
  lseek(tar_fd, -BLOCKSIZE, SEEK_CUR);
  return 0;
}

/* Set mtime of header to actual time */
static void set_hd_time(struct posix_header *hd) {
  time_t now;
  time(&now);
  sprintf(hd -> mtime, "%011o", (unsigned int) now);
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
  }  else if (S_ISFIFO(s -> st_mode)) {
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

static int get_u_and_g_name(struct posix_header *hd, struct stat *s){
  //récupérer le g-name
  struct group *g_name;
  if(s!=NULL)
    g_name = getgrgid(s->st_gid);
  else
    g_name = getgrgid(getgid());
  if(g_name != NULL){
    strncpy(hd->gname, g_name->gr_name, 32);
    hd->gname[31] = '\0';
  }
  //pour récupérer le u-name
  struct passwd *pw;
  uid_t uid;
  if(s == NULL)
    uid = getuid();
  else
    uid = s->st_uid;
  pw = getpwuid (uid);
  if (pw){
    strncpy(hd->uname, pw->pw_name, 32);
    hd->uname[32] = '\0';
  }
  else return -1;

  return 0;
}

static int init_header(struct posix_header *hd, const char *source, const char *filename) {
  struct stat s;
  if (stat(source, &s) < 0) {
    return -1;
  }

  strncpy(hd -> name, filename, 100);
  hd->name[99] ='\0';
  init_mode(hd, &s);
  sprintf(hd -> uid, "%07o", s.st_uid);
  sprintf(hd -> gid, "%07o" ,s.st_gid);
  if(S_ISDIR(s.st_mode)) strcpy(hd -> size, "0");
  else sprintf(hd -> size, "%011lo", s.st_size);
  init_type(hd, &s);
  strcpy(hd -> magic, TMAGIC);
  set_hd_time(hd);
  hd -> version[0] = '0';
  hd -> version[1] = '0';
  get_u_and_g_name(hd, &s);
  set_checksum(hd);
  return 0;
}


static int init_header_empty_file(struct posix_header *hd, const char *filename, int is_dir){
  strncpy(hd -> name, filename, 100);
  hd->name[99] = '\0';
  if(is_dir) sprintf(hd -> mode, "%07o", 0777 & ~getumask());
  else sprintf(hd -> mode, "%07o", 0666 & ~getumask());
  sprintf(hd -> uid, "%07o", getuid());
  sprintf(hd -> gid, "%07o", getgid());
  strcpy(hd -> size, "0");
  set_hd_time(hd);
  hd -> typeflag = (is_dir)? DIRTYPE : REGTYPE;
  strcpy(hd -> magic, TMAGIC);
  hd -> version[0] = '0';
  hd -> version[1] = '0';
  get_u_and_g_name(hd, NULL);
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

static int modif_header(struct posix_header *hd, const char *tar_name_src, const char *source, const char *dest, int fd, struct posix_header *header, int size){
  for(int i = 0; i < size; i++){
    if(strcmp(header[i].name, source) == 0){
      memcpy(hd, &header[i], BLOCKSIZE);
      strcpy(hd->name, dest);
    }
  }
  set_checksum(hd);
  return 0;
}

static int exists_in_tar(const char *name_to_comp, struct posix_header *hd, int size){
  for(int i = 0; i < size; i++){
    if(strcmp(hd[i].name, name_to_comp) == 0){
      return 1;
    }
  }
  return 0;
}

static int read_and_write(int fd_src, int fd_dest, struct posix_header hd){
  //écriture du header
  if (seek_end_of_tar(fd_dest) < 0)
    return -1;
  if (write(fd_dest, &hd, BLOCKSIZE) < 0)
    return -1;

  //écriture du contenu du header
  read_write_buf_by_buf(fd_src, fd_dest, number_of_block(get_file_size(&hd))*BLOCKSIZE, BLOCKSIZE);
  char buffer[BLOCKSIZE];
  memset(buffer, '\0', BLOCKSIZE);
  write(fd_dest, buffer, BLOCKSIZE);
  return 0;
}

int add_tar_file_in_tar_rec(const char *tar_name_src, char *tar_name_dest, const char *source, const char *dest){
  int s = 0;
  struct posix_header *header = tar_ls(tar_name_src, &s);
  int s_2 = 0;
  struct posix_header *header_2 = tar_ls(tar_name_dest, &s_2);

  //check if dont already exist
  if(exists_in_tar(dest, header_2, s_2))
    return error_pt(NULL, 0, errno);

  //We look all the file of tar_name_src
  for(int i = 0; i < s; i++){
    char copy[PATH_MAX];
    int j = 0;

    while(header[i].name[j] == source[j] && j < strlen(source))
    {
      copy[j] = source[j];
      j++;
    }
    copy[j] = '\0';

    //if the copy and source are equals
    if(strcmp(copy, source) == 0 && (header[i].name[j] == '\0' || header[i].name[j-1] == '/')){
      char copy2[PATH_MAX];
      int k = 0;
      for(k = 0; k < strlen(dest); k++){copy2[k] = dest[k];}
      copy2[k] = '\0';
      int l = 0;
      for(l = 0; l < strlen(header[i].name) - strlen(copy); l++){
        copy2[strlen(dest)+l] = header[i].name[strlen(copy)+l];
      }
      copy2[strlen(dest)+l] = '\0';

      //Add Source or a file of source in tar_name_dest as copy2
      add_tar_file_in_tar(tar_name_src, tar_name_dest, header[i].name, copy2);
    }
  }
  free(header);
  free(header_2);
  return 0;
}


int add_tar_file_in_tar(const char *tar_name_src, char *tar_name_dest, const char *source, const char *dest){
  int s = 0;
  struct posix_header *header = tar_ls(tar_name_src, &s);
  int s_2 = 0;
  struct posix_header *header_2 = tar_ls(tar_name_dest, &s_2);
  int tar_src_fd = open(tar_name_src, O_RDWR);
  if ( tar_src_fd < 0)
    return error_pt(NULL, 0, errno);

  //initialisation of the header
  struct posix_header hd;
  memset(&hd, '\0', BLOCKSIZE);
  if(modif_header(&hd, tar_name_src, source, dest, tar_src_fd, header, s) < 0)
    return error_pt(&tar_src_fd, 1, errno);

  //check if it don't already exists
  if(exists_in_tar(dest, header_2, s_2))
    return error_pt(NULL, 0, errno);

  //Check if we want add in the same tar
  if(strcmp(tar_name_src, tar_name_dest) != 0)
  {
    //open of TAR_NAME_DEST
    int tar_dest_fd;
    tar_dest_fd = open(tar_name_dest, O_RDWR);
    if ( tar_dest_fd < 0)
      return error_pt(&tar_src_fd, 1, errno);
    int fds[2] = {tar_dest_fd, tar_src_fd};

    //writing the header and his containing
    if(read_and_write(tar_src_fd, tar_dest_fd, hd) < 0)
      return error_pt(fds, 2, errno);
    add_empty_block(tar_dest_fd);
    close(tar_dest_fd);
  }
  else
  {
    //writing the header and his containing
    if(read_and_write(tar_src_fd, tar_src_fd, hd) < 0)
      return error_pt(&tar_src_fd, 1, errno);
    add_empty_block(tar_src_fd);
  }
  close(tar_src_fd);
  free(header);
  free(header_2);
  return 0;
}

/* Add file SOURCE to tar at path TAR_NAME/FILENAME
   Or create file FILENAME to tar at path TAR_NAME/FILENAME */
int tar_add_file(const char *tar_name, const char *source, const char *filename) {
  int tar_fd = open(tar_name, O_RDWR);
  if ( tar_fd < 0) {
    return error_pt(NULL, 0, errno);
  }
  struct posix_header hd;
  memset(&hd, '\0', BLOCKSIZE);
  if (seek_end_of_tar(tar_fd) < 0) {
    return error_pt(&tar_fd, 1, errno);
  }
  if(source != NULL){
    int src_fd = -1;
    if ((src_fd = open(source, O_RDONLY)) < 0) {
      return error_pt(&tar_fd, 1, errno);
    }
    int fds[2] = {src_fd, tar_fd};
    if(init_header(&hd, source, filename) < 0){
      return error_pt(fds, 2, errno);
    }
    if (write(tar_fd, &hd, BLOCKSIZE) < 0) {
      return error_pt(fds, 2, errno);
    }

    char buffer[BLOCKSIZE];
    ssize_t read_size;
    if(hd.typeflag != DIRTYPE){
      while((read_size = read(src_fd, buffer, BLOCKSIZE)) > 0 ) {
        if (read_size < 0) {
          int fds[2] ={src_fd, tar_fd};
          return error_pt(fds, 2, errno);
        }
        if (read_size < BLOCKSIZE) {
          memset(buffer + read_size, '\0', BLOCKSIZE - read_size);
        }
        if (write(tar_fd, buffer, BLOCKSIZE) < 0) {
          return error_pt(fds, 2, errno);
        }
      }
    }
    add_empty_block(tar_fd);
    close(src_fd);
  }

  else
  {
    if(filename[strlen(filename)-1] == '/')
      init_header_empty_file(&hd, filename, 1);
    else
      init_header_empty_file(&hd, filename, 0);
    if (write(tar_fd, &hd, BLOCKSIZE) < 0) {
      return error_pt(&tar_fd, 1, errno);
    }
  }
  close(tar_fd);
  return 0;
}



/* Append file name FILENAME in tarball TAR_NAME with the content of SRC_FD */
int tar_append_file(const char *tar_name, const char *filename, int src_fd) {
  int tar_fd = open(tar_name, O_RDWR);
  if (tar_fd < 0) {
    return -1;
  }
  struct posix_header hd;
  if (seek_header(tar_fd, filename, &hd) != 1) {
    return error_pt(&tar_fd, 1, ENOENT);
  }
  int size = get_file_size(&hd);
  off_t src_cur = lseek(src_fd, 0, SEEK_CUR);
  off_t src_size = lseek(src_fd, 0, SEEK_END) - src_cur;
  long unsigned int new_size = src_size + size;
  lseek(tar_fd, -BLOCKSIZE, SEEK_CUR);
  set_hd_time(&hd);
  sprintf(hd.size, "%011lo", new_size);
  set_checksum(&hd);
  write(tar_fd, &hd, BLOCKSIZE);
  int padding = BLOCKSIZE - (size % BLOCKSIZE);
  off_t beg = lseek(tar_fd, size, SEEK_CUR);
  off_t tar_size = lseek(tar_fd, 0, SEEK_END);

  int required_blocks = src_size <= padding ? 0 : number_of_block(src_size);
  if (fmemmove(tar_fd, beg + padding, tar_size - (beg + padding), beg + padding + required_blocks*BLOCKSIZE)) {
    close(tar_fd);
    return -1;
  }
  lseek(tar_fd, beg, SEEK_SET);
  lseek(src_fd, src_cur, SEEK_SET);
  if (read_write_buf_by_buf(src_fd, tar_fd, src_size, 512) != 0) {
    close(tar_fd);
    return -1;
  }

  close(tar_fd);
  return 0;
}


int tar_add_file_rec(const char *tar_name, const char *filename, const char *inside_tar_name, int it){
  //it est toujours initialisé à 0;
  if(it++ == 0)tar_add_file(tar_name, filename, inside_tar_name);

  struct dirent *lecture;
  DIR *rep;
  rep = opendir(filename);
  if(rep == NULL){
    closedir(rep);
    return -1;
  }
  //Create a copy of the FILENAME
  char copy[strlen(filename)+1];
  strcpy(copy, filename);
  //While there are files non-explored on the arborescence of FILENAME
  while ((lecture = readdir(rep))) {
    if(strcmp(lecture->d_name, ".") != 0 && strcmp(lecture->d_name, "..") != 0){
      //Copy of the FILENAME and the name of the file discovered in the arborescence in filename
      //It will be the source for tar_add_file
      char *copy2 = malloc(PATH_MAX);
      int i = 0, j = 0;
      for(i = 0; i < strlen(copy); i++)copy2[i] = copy[i];
      if(copy2[i-1] != '/')copy2[i++] = '/';
      for(j = 0; j < strlen(lecture->d_name); j++)copy2[i+j] = lecture->d_name[j];
      if(copy2[i+j-1]!= '/' && lecture->d_type == 4)copy2[i+j++] = '/';
      copy2[i+j] = '\0';

      //a copy of inside_tar_name
      char *copy_inside = malloc(PATH_MAX);
      strcpy(copy_inside, inside_tar_name);
      copy_inside[strlen(inside_tar_name)] = '\0';

      //a copy of inside_tar_name and the name of the file discovered in the arborescence in filename
      //It will be the FILENAME in tar_add_file
      char *copy3 = malloc(PATH_MAX);
      for(i = 0; i < strlen(copy_inside); i++)copy3[i] = copy_inside[i];
      if( i > 0 && copy3[i-1] != '/' && lecture->d_name[0] != '/')copy3[i++] = '/';
      for(j = 0; j < strlen(lecture->d_name); j++)copy3[i+j] = lecture->d_name[j];
      if(copy3[i+j-1]!= '/' && lecture->d_type == 4)copy3[i+j++] = '/';
      copy3[i+j] = '\0';

      tar_add_file(tar_name, copy2, copy3);

      if(lecture->d_type == 4 ){
        tar_add_file_rec(tar_name, copy2, copy3, it);
      }
      free(copy_inside);
      free(copy2);
      free(copy3);
    }
  }
  return 0;
}
