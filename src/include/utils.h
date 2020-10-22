#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <sys/types.h>

/* Read buffer by buffer of size BUFSIZE from READ_FD and write to WRITE_FD up to COUNT. */
int read_write_buf_by_buf(int read_fd, int write_fd, size_t count, size_t bufsize);

int is_dir_name(const char *filename);

int is_prefix(const char *prefix, const char *str);

int fmemmove(int tar_fd, off_t whence, size_t size, off_t where);

#endif
