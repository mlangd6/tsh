#ifndef REDIRECTION_H
#define REDIRECTION_H

typedef enum {
  STDOUT_REDIR,
  STDERR_REDIR,
  STDOUT_APPEND,
  STDERR_APPEND,
  STDIN_REDIR
} redir_type;

typedef struct {
  char *identifier;
  void (*before)(char *);
  void (*after)(char *);
} redir;


void launch_redir_before(redir_type r, char *arg);
void launch_redir_after (redir_type r, char *arg);

#endif
