/**
 * @file errors.h
 * Errors printing
 */
#ifndef ERRORS_H
#define ERRORS_H

#include <stdarg.h>

/**
 * Close files descriptors and set errno
 * @param fds array of file descriptor to close
 * @param length_fds length of `fds`
 * @param new_errno new errno value
 * @return `NULL`
 **/
void *error_p (int fds[], int length_fds, int new_errno);

/**
 * Close files descriptors and set errno
 * @param fds array of file descriptor to close
 * @param length_fds length of `fds`
 * @param new_errno new errno value
 * @return -1
 **/
int error_pt(int fds[], int length_fds, int new_errno);

/**
 * Print command error
 *
 * The error message looks like :
 * `cmd_name: msg: strerror(errno)\n`
 * @param cmd_name a null-terminated string
 * @param msg a null-terminated string
 */
void error_cmd(const char *cmd_name, const char *msg);

/**
 * Print tar command error
 *
 * The error message looks like :
 * `cmd_name: tar_name/filename: strerror(errno)\n`
 * @param cmd_name a null-terminated string
 * @param tar_name a null-terminated string
 * @param filename a null-terminated string
 */
void tar_error_cmd (const char *cmd_name, const char *tar_name, const char *filename);

/**
 * Print error message
 *
 * `msg` use the same format as `printf`
 * See https://en.cppreference.com/w/c/io/fprintf for more details on format
 *
 * @param errnum if different from 0 then `strerror(errnum)\n` is printed at the end
 * @param msg pointer to a null-terminated multibyte string specifying how to interpret the data
 * @param ... arguments specifying data to print
 */
void error (int errnum, const char *msg, ...);

#endif
