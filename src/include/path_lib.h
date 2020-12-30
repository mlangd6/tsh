/**
 * @file path_lib.h
 * Path manipulations
 */

#ifndef PATH_LIB_H
#define PATH_LIB_H

#include <stdbool.h>

/** Type of file in a tar */
enum file_type
  {
    NONE, /**< File doesn't exist */
    DIR,  /**< File is a directory */
    REG   /**< File is not a directory */
  };

/**
 * Split an absolute path after the first tar found
 *
 * In `path`, the `/` following the tar (if there is one) is changed by a `\0` 
 *
 * @examples
 * * `split_tar_abs_path("/tmp") == NULL`
 * * `split_tar_abs_path("/tmp/tsh_test/test.tar") == ""`
 et l'argument est intact
 * * `split_tar_abs_path("/tmp/tsh_test/test.tar/") == ""`
 et l'argument est transformé en : `"/tmp/tsh_test/test.tar"`
 * * `split_tar_abs_path("/tmp/tsh_test/test.tar/man_dir/") == "man_dir/"`
 et l'argument est transformé en : `"/tmp/tsh_test/test.tar"`
 *
 * @param path a null-terminated string representing an absolute path
 * @return a substring of `path` representing the path inside the tar
 */
char *split_tar_abs_path(char *path);

/**
 * Reduce an absolute path
 *
 * If `resolved_path` is `NULL`, then @ref reduce_abs_path uses malloc to allocate a buffer to hold the resolved path,
 * and returns a pointer to this buffer.
 * The caller should deallocate this buffer using free.
 * 
 * @param path an absolute path
 * @param resolved_path if not `NULL` an address to store the reduced path
 * @return the reduced path
 */
char *reduce_abs_path(const char *path, char *resolved_path);

/**
 * Get file type of a file inside a tar
 *
 * @param tar_name path to the tar
 * @param filename path to the file inside `tar_name`
 * @param dir_priority indicates what to prioritize when both types of files exists
 * @return the file type of `filename` inside `tar_name`
 */
enum file_type type_of_file(const char *tar_name, const char *filename, bool dir_priority);

/**
 * Get file type of a file inside a tar
 *
 * Same as @ref type_of_file but uses a file descriptor referencing a tar instead of a path to a tar.
 */
enum file_type ftype_of_file(int tar_fd, const char *filename, bool dir_priority);

/** 
 * Check if a path is a prefix of `PWD`
 *
 * The path must go through a tar
 *
 * @param tar_name path to the tar
 * @param filename path to the file inside `tar_name`
 * @return 0 if the path is a prefix; -1 otherwise
 */
int is_pwd_prefix(const char *tar_name, const char *filename);

/**
 * Get the absolute version of a path
 *
 * The returned path looks like : `PWD/path` (if `path` is not already starting with a `/`)
 *
 * @param path a null-terminated string
 * @return a malloc'd absolute version of `path`
 */
char *make_absolute (const char *path);

/**
 * Check if a path goes through a tar
 *
 * `path` must be absolute
 *
 * @param path a null-terminated string
 * @return 
 * * -1 if `path` is `NULL` or doesn't start with a `/`
 * * 1 if `path` goes through a tar
 * * 0 otherwise
 */
int is_tar_path (char *path);

char *end_of_path(char *path);

#endif
