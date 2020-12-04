#ifndef REDIRECTION_H
#define REDIRECTION_H

typedef enum {
  STDOUT_REDIR,
  STDERR_REDIR,
  STDOUT_APPEND,
  STDERR_APPEND,
  STDIN_REDIR
} redir_type;


void launch_redir_before(redir_type r, char *arg);
void launch_redir_after (redir_type r, char *arg);

#endif
