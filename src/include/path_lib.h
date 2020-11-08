#ifndef PATH_LIB_H
#define PATH_LIB_H

/* Replace '/' after the first tar found by '\0'
    and returns the pointer of the next char */
char *split_tar_abs_path(char *path);

/* Reduce an absolute path (i.e. a path starting with a / ) */
char *reduce_abs_path(const char *path, char *resolved_path);

#endif
