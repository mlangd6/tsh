#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <stdbool.h>

typedef struct unary_command {
  char *name; // Name of command
  int (*in_tar_func) (char *, char *, char *); // Inside tar function to launch (for 1 file)
  bool twd_arg; // Indicates if current working directory should be used if there is no arguments
  bool print_multiple_arg; // Indicates if arguments should be printed before launching function (like in ls)
  char *support_opt; // Supported options for this unary_command
} unary_command;

typedef struct arg_info {
  int nb_tar_file; // Number of arguments that are inside a tarball
  int nb_reg_file; // Number of arguments that are outside a tarball
  int nb_error; // 1 if a file argument is an error (e.g doesn't exists)
  
  size_t options_size; // Number of options
  char **options; // Array containing all the options + unary_command name
} arg_info ;

typedef struct binary_command {
  char *name; // Name of command
  int (*tar_to_tar)(char *src_tar, char *src_file, char *dest_tar, char *dest_file, char *opt);
  int (*extern_to_tar)(char *src_file, char *dest_tar, char *dest_file, char *opt);
  int (*tar_to_extern)(char *src_tar, char *src_file, char *dest_file, char *opt);
  char *support_opt; // Supported options for this unary_command
} binary_command;

/* Handle all the arguments of a unary_command wherever the options are and
   in whatever ordre the arguments (inside/outside tar) are */
int handle_unary_command (unary_command cmd, int argc, char **argv);

int handle_binary_command (binary_command cmd, int argc, char **argv);

#endif
