#ifndef TAR_H
#define TAR_H

/* tar Header Block, from POSIX 1003.1-1990.  */

#define BLOCKSIZE 512
#define BLOCKBITS 9

/* POSIX header.  */

/* Code recupered on https://www.gnu.org/software/tar/manual/html_node/Standard.html and on https://gaufre.informatique.univ-paris-diderot.fr/klimann/systL3_2020-2021/blob/master/TP/TP1/tar.h */
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


/* Add file to tarball */
int tar_add_file(const char *tar_name, const char *filename);

#define OLDGNU_MAGIC "ustar  "  /* 7 chars and a null */


/* List the files contained in a faile .tar */
char **tar_ls(const char *tar_name);

/* Compute and write the checksum of a header */
void set_checksum(struct posix_header *hd);

/* Check that the checksum of a header is correct */
int check_checksum(struct posix_header *hd);

/* Open the tarball TAR_NAME and copy the content of FILENAME into FD */
int tar_read_file(const char *tar_name, const char *filename, int fd);

#endif