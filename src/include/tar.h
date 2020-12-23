/**
 * @file tar.h
 * Tar manipulations
 *
 * All manipulations on tar file are done using low level I/O.
 */

#ifndef TAR_H
#define TAR_H
/* Code taken from https://www.gnu.org/software/tar/manual/html_node/Standard.html and on https://gaufre.informatique.univ-paris-diderot.fr/klimann/systL3_2020-2021/blob/master/TP/TP1/tar.h */

#include <sys/types.h>

#include "array.h"

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

/**
 * Represents a file with its header and data in a tar.
 *
 * Be extremely careful after any changes (mainly write) on #tar_fd as this structure may not stay coherent.
 */
typedef struct
{
  int tar_fd;                 /**< a file descriptor referencing the tar owning this file */
  struct posix_header header; /**< the posix header for this file */
  off_t file_start;           /**< the beginning of the header of this file in #tar_fd */

} tar_file;


/* Compute and write the checksum of a header, by adding all (unsigned) bytes in
   it (while hd->chksum is initially all ' ').
   Then hd->chksum is set to contain the octal encoding of this sum (on 6 bytes), followed by '\0' and ' ' */
void set_checksum(struct posix_header *hd);

/* Check that the checksum of a header is correct.
   Return :
   1 if header is correct
   0 otherwise */
int check_checksum(struct posix_header *hd);

/* Check if the file at PATH is a valid tar.
   Return :
   1  if all headers are correct
   0  if at least one header is invalid or can't read a full block
   -1 otherwise */
int is_tar(const char *path);

/* Seek FILENAME in the tar referenced by TAR_FD and set HEADER accordingly.
   Return :
   1  if FILENAME is in the tar and set HEADER for this file
   0  if FILENAME is not in the tar
   -1 if there is any kind of error

   In any case, the file offset of TAR_FD is moved and memory area at HEADER is changed.
   If FILENAME was found (i.e. return 1) then this function guarantees that :
   - the file offset is moved to the end of the header *and*
   - HEADER is correctly set for the wanted file */
int seek_header(int tar_fd, const char *filename, struct posix_header *header);

/* Convert FILESIZE into a number of blocks */
unsigned int number_of_block(unsigned int filesize);

/* Return the file size from a given header */
unsigned int get_file_size(const struct posix_header *hd);

/* Increment the file offset of TAR_FD by file size given in HD.
   This function is intended to be use after reading a header in a tar, when the file offset is moved to the end of HD header.
   Return :
   On success, the offset location as measured in bytes from the beginning of the file
   -1 otherwise */
int skip_file_content(int tar_fd, struct posix_header *hd);



/* Add file at path FILENAME to tar at path TAR_NAME
   Return :
   0  if FILENAME was added
   -1 if it couldn't */
int tar_add_file(const char *tar_name, const char *source, const char *filename);

/* Add a directory filename to tar at path inside_tar_name with all that he contains
   IT is here just for the first iteration
   Return :
   0 if FILENAME and his containing were added
  -1 if they couldn't */
int tar_add_file_rec(const char *tar_name, const char *filename, const char *inside_tar_name, int it);

/* Return the number of files in the tar referenced by TAR_FD */
int nb_files_in_tar(int tar_fd);

/* Return the number of files in the tar TAR_NAME */
int nb_files_in_tar_c(char *tar_name);


struct posix_header *tar_ls(const char *tar_name, int *size);

array* tar_ls_dir (int tar_fd, const char *dir_name, bool rec);

array* tar_ls_all (int tar_fd);


int tar_cp_file(const char *tar_name, const char *filename, int fd);

int tar_extract (const char *tar_name, const char *filename, const char *dest);

/* Open the tarball at path TAR_NAME and delete FILENAME if possible
   Return
   0  if FILENAME was successfully deleted from the tar
   -1 if a system call failed
   -2 for other errors such as : FILENAME not in tar, FILENAME is a directory not finishing with '/'... */
int tar_rm(const char *tar_name, const char *filename);

/* Open the tar at path TAR_NAME and copy the content of FILENAME into FD then delete FILENAME from the tar
   Return :
   0  if FILENAME was found and moving the content was done without any issue
   -1 if FILENAME was not found or is not a regular file or a system call failed */
int tar_mv_file(const char *tar_name, const char *filename, int fd);

/* Check user's permissions for file FILE_NAME in tar at path TAR_NAME

   The  mode  specifies the accessibility check(s) to be performed, and is the value F_OK
   F_OK tests for the existence of the file.

   RETURN :
   On success, 1 is returned if FILE_NAME was exactly found in the tar, or 2 is returned if FILE_NAME is finishing with a / (i.e. is a directory) and was found existing only through its subfiles.
   On error, -1 is returned and errno is set appropriately.

   ERRORS :
   ENOENT A component of FILE_NAME does not exist
   EINVAL MODE was incorrectly specified. */
int tar_access(const char *tar_name, const char *file_name, int mode);

int ftar_access(int tar_fd, const char *file_name, int mode);

/* Append file name FILENAME in tarball TAR_NAME with the content of SRC_FD
   Return :
   0 if everything worked
   -1 if a system call failed */
int tar_append_file(const char *tar_name, const char *filename, int src_fd);

/* Add a file SOURCE from TAR_NAME_SRC to TAR_NAME_DEST with the name of file DEST
   Return :
   0 if FILENAME and his containing were added
  -1 if they couldn't */
int add_tar_file_in_tar(const char *tar_name_src, char *tar_name_dest, const char *source, const char *dest);

/* Add a file SOURCE from TAR_NAME_SRC to TAR_NAME_DEST with the name of file DEST
   If the file is a directory and have other files in him, they are also add in the
   TAR_NAME_DEST in the directory add before
   Return :
   0 if FILENAME and his containing were added
  -1 if they couldn't */
int add_tar_file_in_tar_rec(const char *tar_name_src, char *tar_name_dest, const char *source, const char *dest);

array* tar_ls_if (int tar_fd, bool (*predicate)(const struct posix_header *));
/* Set mtime of header to actual time */
void set_hd_time(struct posix_header *hd);

/**
 * Update header of file inside tarball
 * @param tar_fd is the file descriptor of the tarball
 * @param filename is the name of the file inside the tarball
 * @param update is the fonction that updates the posix_header
 * After the update of the header the checksum will be re calculated and
 * the header mtime will me set to current time
 * @return 0 on success, else -1
 */
int update_header(struct posix_header *hd, int tar_fd, char *filename, void (*update)(struct posix_header *hd));

int move_file_to_end_of_tar(char *tar_name, char *filename);

/* Check if filename is the name of a directory in the tar.
   If true then return 1 else 0 */
int is_dir(const char *tar_name, const char *filename);

int tar_rm_dir(int tar_fd, const char *dirname);


#endif
