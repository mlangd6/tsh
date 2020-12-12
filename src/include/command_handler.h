#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <stdbool.h>

typedef struct command {
  char *name; // Name of command
  int (*in_tar_func) (char *, char *, char *); // Inside tar function to launch (for 1 file)
  short twd_arg; // Indicates if current working directory should be used if there is no arguments
  short print_multiple_arg; // Indicates if arguments should be printed before launching function (like in ls)
  char *support_opt; // Supported options for this command
} command;

typedef struct arg_info {
  int nb_in_tar; // Number of arguments that are inside a tarball
  int nb_out; // Number of arguments that are outside a tarball
  bool err_arg; // 1 if a file argument is an error (e.g doesn't exists)
  int opt_c; // Number of options
  char **options; // Array containing all the options
} arg_info ;

/* Handle all the arguments of a command wherever the options are and
  in whatever ordre the arguments (inside/outside tar) are */
int handle(command c, int argc, char **argv);

#endif
