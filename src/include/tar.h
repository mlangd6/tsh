/**
 * @file tar.h
 * Tar manipulations
 *
 * All manipulations on tar file are done using low level I/O.
 */

#ifndef TAR_H
#define TAR_H


#include <sys/types.h>

#include "array.h"

/* Code taken from :
 * https://www.gnu.org/software/tar/manual/html_node/Standard.html
 * https://gaufre.informatique.univ-paris-diderot.fr/klimann/systL3_2020-2021/blob/master/TP/TP1/tar.h */

/** tar Header Block size in bytes, from POSIX 1003.1-1990 */
#define BLOCKSIZE 512
#define BLOCKBITS 9


/** 
 * POSIX header
 * 
 * All characters in header blocks are represented by using 8-bit characters in the local variant of ASCII.
 * Each field within the structure is contiguous; that is, there is no padding used within the structure.
 * Each character on the archive medium is stored contiguously.
 * 
 * Bytes representing the contents of files (after the header block of each file) are not translated in any way and are not constrained to represent characters in any character set.
 * The tar format does not distinguish text files from binary files, and no translation of file contents is performed.
 * 
 * The `name`, `linkname`, `magic`, `uname`, and `gname` are null-terminated character strings.
 * All other fields are zero-filled octal numbers in ASCII.
 * Each numeric field of width w contains w minus 1 digits, and a null. (In the extended GNU format, the numeric fields can take other forms.) 
 */
struct posix_header
{                              
  char name[100];     /**< file name of the file, with directory names (if any) preceding the file name, separated by slashes. */
  char mode[8];       /**< nine bits specifying file permissions and three bits to specify the Set UID, Set GID, and Save Text (sticky) modes. */         
  char uid[8];        /**< numeric user ID of the file owners */
  char gid[8];        /**< numeric group ID of the file owners */
  char size[12];      /**< size of the file in bytes */
  char mtime[12];     /**< data modification time of the file at the time it was archived. It represents the integer number of seconds since January 1, 1970, 00:00 UTC. */    
  char chksum[8];     /**< simple sum of all bytes in the header block */     
  char typeflag;      /**< type of file archived */
  char linkname[100]; /**< the file targeted, if the file is a link */
  char magic[6];               
  char version[2];             
  char uname[32];     /**< user name of the file owners */
  char gname[32];     /**< group name of the file owners */         
  char devmajor[8];            
  char devminor[8];            
  char prefix[155];            
  char junk[12];               
};                             

#define TMAGIC   "ustar"        /**< ustar and a null */
#define TMAGLEN  6
#define TVERSION "00"           /**< 00 and no null */
#define TVERSLEN 2

#define REGTYPE  '0'            /**< regular file */
#define AREGTYPE '\0'           /**< regular file */
#define LNKTYPE  '1'            /**< link */
#define SYMTYPE  '2'            /**< reserved */
#define CHRTYPE  '3'            /**< character special */
#define BLKTYPE  '4'            /**< block special */
#define DIRTYPE  '5'            /**< directory */
#define FIFOTYPE '6'            /**< FIFO special */
#define CONTTYPE '7'            /**< reserved */

#define OLDGNU_MAGIC "ustar  "  /**< 7 chars and a null */

/* Bits used in the mode field, values in octal.  */
#define TSUID    04000          /* set UID on execution */
#define TSGID    02000          /* set GID on execution */
#define TSVTX    01000          /* reserved */
                                /* file permissions */
#define TUREAD   00400          /* read by owner */
#define TUWRITE  00200          /* write by owner */
#define TUEXEC   00100          /* execute/search by owner */
#define TGREAD   00040          /* read by group */
#define TGWRITE  00020          /* write by group */
#define TGEXEC   00010          /* execute/search by group */
#define TOREAD   00004          /* read by other */
#define TOWRITE  00002          /* write by other */
#define TOEXEC   00001          /* execute/search by other */

/**
 * File with its header and data in a tar.
 *
 * Be extremely careful after any changes (mainly write) on #tar_fd as this structure may not stay coherent.
 */
typedef struct
{
  int tar_fd;                 /**< a file descriptor referencing the tar owning this file */
  struct posix_header header; /**< the posix header for this file */
  off_t file_start;           /**< the beginning of the header of this file in #tar_fd */

} tar_file;


/**
 * Update the checksum field of a header
 *
 * Compute and write the checksum of a header, by adding all (unsigned) bytes in it ( `while hd->chksum` is initially all ' ' ).
 * Then `hd->chksum` is set to contain the octal encoding of this sum (on 6 bytes), followed by '\0' and ' '.
 * @param hd the header whose checksum field must be updated
 */
void set_checksum (struct posix_header *hd);

/**
 * Check the checksum field of a header
 *
 * @param hd the header whose checksum field must be checked
 * @return 1 if the computed checksum is correct; 0 otherwise 
 */
int check_checksum (struct posix_header *hd);

/**
 * Check if a file is a valid tar
 *
 * @param path the file to check
 * @return
 * * 1 if all headers are correct
 * * 0 if at least one header is invalid or can't read a full block
 * * -1 otherwise
 */
int is_tar (const char *path);

/**
 * Seek a header in a tar
 *
 * Seeks `filename` in the tar referenced by `tar_fd` and set `header` accordingly.
 * Seeking occurs at the current offset.
 * 
 * @param tar_fd the file descriptor of the tar to look in
 * @param filename the file name to seek in the tar
 * @param header the address to store the result
 * @return
 * * 1 if `filename` is in the tar and set `header` for this file
 * * 0 if `filename` is not in the tar
 * * -1 if there is any kind of error
 *
 * In any case, the file offset of `tar_fd` is moved and memory area at `header` is changed.
 * If `filename` was found (i.e. returns 1) then this function guarantees that :
 * * the file offset is moved to the end of the header **and**
 * * `header` is correctly set for the wanted file 
 */
int seek_header (int tar_fd, const char *filename, struct posix_header *header);

/**
 * Converts a positive integer to a number of blocks
 * @param filesize the integer to be converted
 * @return `filesize` converted in blocks
 */
unsigned int number_of_block(unsigned int filesize);

/**
 * Gets the file size in base 10 from a posix header
 * @param hd a pointer to a posix header from which the size must be read
 * @return the file size read
 */
unsigned int get_file_size(const struct posix_header *hd);

/**
 * Skip file content in a tar 
 *
 * Move the file offset of `tar_fd` by file size given in a posix_header.
 * This function is intended to be used after reading a header in a tar, when the file offset is at the end of `hd`.
 *
 * @param tar_fd a file descriptor referencing a tar
 * @param hd file header whose content must be skipped
 * @return on success, the offset location as measured in bytes from the beginning of the file; -1 otherwise 
 */
int skip_file_content(int tar_fd, struct posix_header *hd);

/**
 * Return the number of files in a tar referenced by a file descriptor
 *
 * @param tar_fd a file descriptor referencing a tar
 * @return the number of file in the tar referenced by `tar_fd`
 */
int nb_files_in_tar(int tar_fd);

/**
 * Return the number of files in a tar
 *
 * @param tar_name path to the tar
 * @return the number of file in the tar at path `tar_name`
 */
int nb_files_in_tar_c(char *tar_name);

/**
 * List all files contained in a tar
 *
 * @param tar_name path to the tar to list
 * @param size an address to store the size of the returned array
 * @return on success, a malloc'd array of all headers; NULL on failure
 */
struct posix_header *tar_ls(const char *tar_name, int *size);

/**
 * Lists all files from a directory contained in a tar
 *
 * The file descriptor `tar_fd` must be already opened with at least read mode and `dir_name` must be a null terminated string.
 * To list files at root just put an empty string for `dir_name` otherwise make sure `dir_name` ends with a `/`.
 *
 * Note that, the returned array has no entry for the listed directory (i.e. `dir_name`).
 *
 * @param tar_fd a file descriptor referencing a tar
 * @param dir_name the directory to list
 * @param rec if `true` then subdirectories are also listed
 * @return a malloc'd pointer to an array of tar_file; `NULL` if there are errors
 */
array* tar_ls_dir (int tar_fd, const char *dir_name, bool rec);

/**
 * Lists all files in a tar
 *
 * The file descriptor `tar_fd` must be already opened with at least read mode.
 *
 * @param tar_fd the file descriptor referencing a tar
 * @return a malloc'd pointer to an array of tar_file; `NULL` if there are errors
 */
array* tar_ls_all (int tar_fd);

/**
 * Lists all files passing a predicate in a tar.
 *
 * @param tar_fd the file descriptor referencing a tar
 * @param predicate a predicate for a posix_header
 * @return a malloc'd pointer to an array of tar_file; `NULL` if there are errors
 */
array* tar_ls_if (int tar_fd, bool (*predicate)(const struct posix_header *));

/**
 * Read the content of a file from a tar and write it to a file descriptor
 *
 * Copy the content of a file from a tar into a file descriptor.
 * filename should be readable
 * @param tar_name the tar in which we want to read `filename`
 * @param filename the file we want to read
 * @param fd a file descriptor to write to
 * @return 0 on success; -1 otherwise
 */
int tar_cp_file(const char *tar_name, const char *filename, int fd);

/**
 * Extract a file from a tar.
 *
 * `dest` must designate an already existing directory.
 * If `filename` is a directory then files from `dir_name` are extracted in `dest/filename/`.
 * Otherwise `filename` is extracted to `dest/`
 *
 * @param tar_name the path to the tar
 * @param filename the file to extract from the tar
 * @param dest the path to the output directory
 * @return on success 0; otherwise -1
 */
int tar_extract (const char *tar_name, const char *filename, const char *dest);

/**
 * Remove a file from a tar
 *
 * If `filename` is an empty string or ends with a `/` then `filename` is removed recursively as a directory.\n
 * Else `filename` is removed as a regular file.
 *
 * @param tar_name the path to the tar
 * @param filename the file to remove from the tar
 * @return
 * *  0 if `filename` was successfully deleted from the tar
 * * -1 if a system call failed
 * * -2 for other errors such as : FILENAME not in tar, FILENAME is a directory not finishing with '/'... 
 */
int tar_rm(const char *tar_name, const char *filename);

/** 
 * Remove a directory recursively from a tar
 * 
 * `filename` must be an empty string or must end with a `/`.
 *
 * @param tar_fd the file descriptor referencing the tar
 * @param dirname path to the directory we want to delete in the tar
 * @return 0 on success, -1 if a system call failed
 */
int tar_rm_dir(int tar_fd, const char *dirname);

/**
 * Read the content of a file from a tar and write it to a file descriptor, then remove it from the tar
 *
 * @param tar_name path to the tar in which we want to read `filename`
 * @param filename path to the file we want to read in `tar_name`
 * @param fd a file descriptor to write to
 * @return 0 on success; -1 otherwise
 */
int tar_mv_file(const char *tar_name, const char *filename, int fd);

/**
 * Check user's permissions for file in a tar
 *
 * The mode specifies the accessibility check(s) to be performed, and is either the value F_OK,
 * or a mask consisting of the bitwise OR of one or more of R_OK, W_OK, and X_OK. 
 * F_OK tests for the existence of the file.
 * R_OK, W_OK, and X_OK test whether the file exists and grants read, write, and execute permissions, respectively.
 *
 * @param tar_name path to the tar
 * @param file_name path to the file in `tar_name`
 * @param mode explained above
 * @return
 * * On success (all requested permissions granted, or mode is F_OK and the file exists)
 *   * 1 if `file_name` was exactly found in the tar
 *   * 2 if `file_name` is finishing with a `/` (i.e. is a directory) and was found existing only through its subfiles.
 *
 * * On error (at least one bit in mode  asked for a permission that is denied, or mode is F_OK and the file does not exist, or some other error occurred), 
 *   * -1 is returned, and errno is set appropriately.
 *
 * ERRORS :
 * ENOENT A component of FILE_NAME does not exist
 * EINVAL MODE was incorrectly specified. 
 */
int tar_access(const char *tar_name, const char *file_name, int mode);

/**
 * Check user's permissions for file in a tar
 *
 * Same as @ref tar_access but uses a file descriptor referencing a tar instead of a path to a tar.
 */
int ftar_access(int tar_fd, const char *file_name, int mode);

/**
 * Add an extern file to a tar
 *
 * The insertion takes place at the end of the tar.
 *
 * If `source != NULL` then file at path `source` is inserted in `tar_name` as `filename`.\n
 * Else `tar_name/filename` is created.
 * Moreover, if `filename` is ending with a `/` then `tar_name/filename` is created as a directory instead of a regular file.
 *
 * @param tar_name path to the tar
 * @param source path to the file to add, or `NULL`
 * @param filename name inside the tar
 * @return 0 if `filename` was added; -1 if not
 */
int add_ext_to_tar(const char *tar_name, const char *source, const char *filename);

/**
 * Add an extern directory recursively to a tar
 *
 * The insertion takes place at the end of the tar.
 * `filename` is inserted in `tar_name` as `inside_tar_name`
 *
 * @param tar_name path to the tar
 * @param filename path to the directory to insert recursively
 * @param inside_tar_name name inside the tar
 * @param it must be set to 0
 * @return 0 if `filename` was recursively added; -1 if not
 */
int add_ext_to_tar_rec(const char *tar_name, const char *filename, const char *inside_tar_name, int it);

/**
 * Add a file from a tar to a tar
 *
 * `tar_name_src` and `tar_name_dest` can designate the same path to a tar.
 * There must be no file named `dest` in `tar_name_dest`.
 *
 * @param tar_name_src path to the source tar
 * @param tar_name_dest path to the dest tar
 * @param source path to the file (in `tar_name_src`) we want to copy
 * @param dest destination name (in `tar_name_dest`)
 * @return 0 on success; -1 on error
 */
int add_tar_to_tar(const char *tar_name_src, char *tar_name_dest, const char *source, const char *dest);

/**
 * Add a directory recursively from a tar to a tar
 *
 * `tar_name_src` and `tar_name_dest` can designate the same path to a tar.
 * There must be no file named `dest` in `tar_name_dest`.
 *
 * @param tar_name_src path to the source tar
 * @param tar_name_dest path to the dest tar
 * @param source path to the file (in `tar_name_src`) we want to copy
 * @param dest destination name (in `tar_name_dest`)
 * @return 0 on success; -1 on error
 */
int add_tar_to_tar_rec(const char *tar_name_src, char *tar_name_dest, const char *source, const char *dest);

/**
 * Append content to a file in a tar
 *
 * @param tar_name path to the tar
 * @param filename the file we want to extend
 * @param src_fd the file descriptor in which we read from
 * @return 0 if everything worked; -1 if a system call failed 
 */
int tar_append_file(const char *tar_name, const char *filename, int src_fd);

/**
 * Set mtime of a header to actual time
 * @param hd pointer to the posix_header that needs to be updated
 */
void set_hd_time(struct posix_header *hd);

/**
 * Update a posix header
 *
 * After the update of the header the checksum will be re-calculated and
 * the field `mtime` of the will be set to current time
 *
 * @param hd an address to store the updated header
 * @param tar_fd the file descriptor referencing the tar
 * @param filename the name of the file inside the tar
 * @param update is the fonction that updates the posix_header
 * @return 0 on success, else -1
 */
int update_header(struct posix_header *hd, int tar_fd, char *filename, void (*update)(struct posix_header *hd));

/**
 * Move a file from a tar to the end
 *
 * The header and the content of the file is moved to the end
 *
 * @param tar_name path to the tar
 * @param filename path to the file in `tar_name`
 * @return 0 on success, -1 else
 */
int move_file_to_end_of_tar(char *tar_name, char *filename);

/**
 * Check if a file is a directory in a tar
 * @param tar_name path to the tar
 * @param filename path to the file in `tar_name`
 * @return 1 if `tar_name/filename` is a directory else 0
 */
int is_dir(const char *tar_name, const char *filename);

#endif
