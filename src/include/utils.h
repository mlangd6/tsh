#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <sys/types.h>

/* return the umask */
mode_t getumask(void);

/* Read buffer by buffer of size BUFSIZE from READ_FD and write to WRITE_FD up to COUNT bytes.
   On success, 0 is returned.
   On error,  -1 is returned. */
int read_write_buf_by_buf(int read_fd, int write_fd, size_t count, size_t bufsize);


/* Check if FILENAME ends with '/'.
   If true then return 1 else 0. */
int is_dir_name(const char *filename);


/* Check if STR starts with PREFIX.
   If true then return 1 else 0*/
int is_prefix(const char *prefix, const char *str);


/* Copies SIZE bytes from file descriptor FD starting at WHENCE offset to WHERE offset.
   The memory areas may overlap.
   At the end of the operation, the file offset is set to WHERE.
   On success, 0 is returned.
   On error,  -1 is returned. */
int fmemmove(int fd, off_t whence, size_t size, off_t where);

#endif
