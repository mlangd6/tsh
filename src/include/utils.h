/**
 * @file utils.h
 * Utility functions
 */

#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <sys/types.h>

/**
 * Get file creation mask
 * @return the current file creation mask
 */
mode_t getumask(void);

/**
 * Read and write buffer by buffer
 *
 * Read buffer by buffer of size `bufsize` from `read_fd` and write to `write_fd` up to `count` bytes.
 *
 * @param read_fd the file descriptor to read from
 * @param write_fd the file descriptor to write to
 * @param count number of bytes to read from `read_fd`
 * @param bufsize desired buffer size
 * @return 0 on success; -1 otherwise
 */
int read_write_buf_by_buf(int read_fd, int write_fd, size_t count, size_t bufsize);

/**
 * Check if a string ends with a `/`
 * @param str a null-terminated string
 * @return 1 if ̀`filename` ends with a `/`; 0 otherwise
 */
int is_dir_name(const char *str);

/**
 * Tests if a string starts with a specified prefix.
 *
 * Strings must be null-terminated.
 *
 * @param prefix the prefix
 * @param str the string to test
 * @return 
 * * 1 if `prefix` is indeed a prefix of `str`
 * * 2 if `str` and `prefix` are equals
 * * 0 otherwise
 */
int is_prefix(const char *prefix, const char *str);

/**
 * Check if a string is empty
 * @param str a null-terminated string
 * @return 1 if `str` is empty; 0 otherwise
*/
int is_empty_string(const char *str);

/**
 * Move a range of data from a file descriptor
 *
 * Copy `size` bytes from file descriptor `fd` starting at `whence` offset to `where` offset.
 * The memory areas may overlap. At the end of the operation, the file offset is moved to `where`.
 *
 * @param fd a file descriptor
 * @param whence an offset in `fd` to read from
 * @param size number of bytes to copy
 * @param where an offset in `fd` to write from
 * @return 0 on success; -1 otherwise
 */
int fmemmove(int fd, off_t whence, size_t size, off_t where);

/** 
 * Write a string to a file descriptor.
 * @param fd a file descriptor to write to
 * @param str a null-terminated string
 * @return On success, the number of bytes written is returned. On error, -1.
 */
int write_string (int fd, const char *str);

/**
 * Copy a string
 * @param str a null-terminated string
 * @return a malloc'd copy of `str`
 */
char *copy_string (const char *str);

/**
 * Append `/` to a string
 * A trailing `/` is added if `str` doesn't already end with a `/`.
 * @param str a null-terminated string
 * @return on success, a malloc'd copy of `str` with a `/` if possible; `NULL` otherwise
  */
char *append_slash(const char *str);

/**
  * remove '/' at the end of a string if it ends by '/'
  * @param s the string
  */
void remove_last_slash(char *s);

#endif
