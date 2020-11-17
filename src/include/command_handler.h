#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

typedef struct command {
  char *name;
  int (*in_tar_func) (char *, char *, char *);
  short twd_arg;
  short print_multiple_arg;
  char *support_opt;
} command;

typedef struct arg_info {
  int nb_in_tar;
  int nb_out;
  int opt_c;
  char **options;
} arg_info ;

int handle(command c, int argc, char **argv);

#endif
