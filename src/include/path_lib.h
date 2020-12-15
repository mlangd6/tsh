#ifndef PATH_LIB_H
#define PATH_LIB_H

#include <stdbool.h>

enum file_type {
  NONE, // File dosen't exists
  DIR, // Directory
  REG // file that is not a directory
};

/* Replace '/' after the first tar found by '\0'
    and returns the pointer of the next char */
char *split_tar_abs_path(char *path);

/* Reduce an absolute path (i.e. a path starting with a / ) */
char *reduce_abs_path(const char *path, char *resolved_path);
/**
 * Return the file type of filename
 * @param tar_name a name of a tarball
 * @param filename the name of the file inside the tarball
 * @param dir_priority indicates what to prioritize when both types of files exists
 */
enum file_type type_of_file(const char *tar_name, const char *filename, bool dir_priority);

#endif
