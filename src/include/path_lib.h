#ifndef PATH_LIB_H
#define PATH_LIB_H

/* Replace '/' after the first tar found by '\0'
    and returns the pointer of the next char */
char *split_tar_abs_path(char *path);

/* Replace all the . and .. to get the real abs path of file
  This function use malloc() and the return value should be free() */
char *reduce_abs_path(char const *path);

#endif
