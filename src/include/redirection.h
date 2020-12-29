#ifndef REDIRECTION_H
#define REDIRECTION_H

typedef enum {
  STDOUT_REDIR,
  STDERR_REDIR,
  STDOUT_APPEND,
  STDERR_APPEND,
  STDIN_REDIR
} redir_type;


int launch_redir(redir_type r, char *arg);
void init_redirections();
void reset_redirs();
void exit_redirections();
void add_reset_redir(int fd, pid_t pid);


#endif
