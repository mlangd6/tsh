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
unsigned int get_file_size(struct posix_header *hd);

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

/* List all files contained in the tar at path TAR_NAME
   Return :
   On success, a malloc array of all headers
   On failure, NULL */
struct posix_header *tar_ls(const char *tar_name, int *size);

/* Open the tar at path TAR_NAME and copy the content of FILENAME into FD
   Return :
   0  if FILENAME was found and the copy was done without any issue
   -1 if FILENAME was not found or is not a regular file or a system call failed */
int tar_cp_file(const char *tar_name, const char *filename, int fd);

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
   On success, 1 is returned if FILE_NAME was exactly found in hte tar, or 2 is returned if FILE_NAME is finishing with a / (i.e. is a directory) and was found existing only through its subfiles.
   On error, -1 is returned and errno is set appropriately.

   ERRORS :
   ENOENT A component of FILE_NAME does not exist
   EINVAL MODE was incorrectly specified. */
int tar_access(const char *tar_name, const char *file_name, int mode);

/* Append file name FILENAME in tarball TAR_NAME with the content of SRC_FD
   Return :
   0 if everything worked
   -1 if a system call failed */
int tar_append_file(const char *tar_name, const char *filename, int src_fd);

/* Return 1 if at least one argument of ARGV implies a tarball and 0 else */
int has_tar_arg(const char **argv, int argc);
#endif
