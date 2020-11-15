#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

typedef struct command {
  char *name;
  int (*in_tar_func) (char *, char *, char *);
  int twd_arg;
  char *support_opt;
} command;

int handle(command c, int argc, char **argv);

/* Return 1 if at least one argument of ARGV implies a tarball
   2 if there is no argument (other than options)
   0 else */
int has_tar_arg(int argc, char **argv);

#endif
