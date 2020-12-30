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

/**
 * Return the file type of filename
 * @param tar_fd the file descriptor of a tarball
 * @param filenale the name of the file inside the tarball
 * @param dir_priority indicates what to prioritize when both types of files exists
 */
enum file_type ftype_of_file(int tar_fd, const char *filename, bool dir_priority);

/** Return 0 if pwd is prefix of an inside tarball path
 * @param tar_name the name of the tarball
 * @param filename the name of the file inside the tarball
 * @return 0 if true, else -1
 */

int is_pwd_prefix(const char *tar_name, const char *filename);
/**
 * Gets the absolute version of a path.
 */
char *make_absolute (const char *path);

/**
 * Checks if an absolute path goes through a tar.
 */
int is_tar_path (char *path);

char *end_of_path(char *path);

#endif
