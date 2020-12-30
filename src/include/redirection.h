/**
 * @file redirection.h
 * Manipulation of redirections.
 */

#ifndef REDIRECTION_H
#define REDIRECTION_H

/**
 * The differents type of redirections.
 */
typedef enum {
  STDOUT_REDIR, /**< ">" redirection */
  STDERR_REDIR, /**< "2>" redirection */
  STDOUT_APPEND, /**< ">>" redirection */
  STDERR_APPEND, /**< "2>>" redirection */
  STDIN_REDIR /**< "<" redirection */
} redir_type;

/**
 * Launch one redirection.
 * @param r The type of the redirection.
 * @param arg The redirection file.
 * @return 0 on success, -1 else.
 */
int launch_redir(redir_type r, char *arg);

/**
 * Init the redirections.
 */
void init_redirections();

/**
 * reset all the ongoing redirections.
 */
void reset_redirs();

/**
 * Exit the redirections.
 */
void exit_redirections();

/**
 * Add a reset of redirection.
 * @param fd The file descriptor which will be redirected
 * @param pid The child process which takes care of the  redirections, 0 if no
 * child process is used.
 */
void add_reset_redir(int fd, pid_t pid);


#endif
