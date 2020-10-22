#ifndef TAR_H
#define TAR_H
/* Code taken from https://www.gnu.org/software/tar/manual/html_node/Standard.html and on https://gaufre.informatique.univ-paris-diderot.fr/klimann/systL3_2020-2021/blob/master/TP/TP1/tar.h */

/* tar Header Block, from POSIX 1003.1-1990.  */
#define BLOCKSIZE 512
#define BLOCKBITS 9

/* POSIX header.  */
struct posix_header
{                              /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[6];                /* 257 */
  char version[2];              /* 263 */
  char uname[32];               /* 265 */
  char gname[32];               /* 297 */
  char devmajor[8];             /* 329 */
  char devminor[8];             /* 337 */
  char prefix[155];             /* 345 */
  char junk[12];                /* 500 */
};                              /* Total: 512 */

#define TMAGIC   "ustar"        /* ustar and a null */
#define TMAGLEN  6
#define TVERSION "00"           /* 00 and no null */
#define TVERSLEN 2

#define REGTYPE  '0'            /* regular file */
#define AREGTYPE '\0'           /* regular file */
#define LNKTYPE  '1'            /* link */
#define SYMTYPE  '2'            /* reserved */
#define CHRTYPE  '3'            /* character special */
#define BLKTYPE  '4'            /* block special */
#define DIRTYPE  '5'            /* directory */
#define FIFOTYPE '6'            /* FIFO special */
#define CONTTYPE '7'            /* reserved */

#define OLDGNU_MAGIC "ustar  "  /* 7 chars and a null */


/* Compute and write the checksum of a header */
void set_checksum(struct posix_header *hd);

/* Check that the checksum of a header is correct */
int check_checksum(struct posix_header *hd);

/* Check if the file at PATHNAME is a valid tarball */
int is_tar(const char *tar_name);

int find_header(int tar_fd, const char *filename, struct posix_header *header);

unsigned int number_of_block(unsigned int filesize);

unsigned int get_file_size(struct posix_header *hd);

/* If it succeed returns the number of bytes from the beginning of the file, cursor of tar_fd is moved. Otherwise, -1 */
int skip_file_content(int tar_fd, struct posix_header *hd);

  

/* Add file to tarball */
int tar_add_file(const char *tar_name, const char *filename);

/* List the files contained in a faile .tar */
struct posix_header *tar_ls(const char *tar_name);

/* Open the tarball TAR_NAME and copy the content of FILENAME into FD */
int tar_cp_file(const char *tar_name, const char *filename, int fd);

/* Open the tarball at path TAR_NAME and delete FILENAME.
   Returns :
   0  if it succeed
   -1 if a system call failed
   -2 for other errors such as : FILENAME not in tar, FILENAME is a directory not finishing with '/'... */
int tar_rm(const char *tar_name, const char *filename);

/* Open the tarball TAR_NAME and copy the content of FILENAME into FD then delete FILENAME */
int tar_mv_file(const char *tar_name, const char *filename, int fd);

#endif
