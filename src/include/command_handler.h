#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <stdbool.h>

typedef struct unary_command
{
  char *name; // Name of command
  int (*in_tar_func) (char *, char *, char *); // Inside tar function to launch (for 1 file)
  bool twd_arg; // Indicates if current working directory should be used if there is no arguments
  bool print_multiple_arg; // Indicates if arguments should be printed before launching function (like in ls)
  char *support_opt; // Supported options for this unary_command
} unary_command;

typedef struct arg_info
{
  int nb_tar_file; // Number of arguments that are inside a tarball
  int nb_reg_file; // Number of arguments that are outside a tarball
  int nb_error; // 1 if a file argument is an error (e.g doesn't exists)
  
  size_t options_size; // Number of options
  char **options; // Array containing all the options + unary_command name
} arg_info ;

typedef struct binary_command
{
  char *name; // Name of command
  int (*tar_to_tar)(char *src_tar, char *src_file, char *dest_tar, char *dest_file, char *opt);
  int (*extern_to_tar)(char *src_file, char *dest_tar, char *dest_file, char *opt);
  int (*tar_to_extern)(char *src_tar, char *src_file, char *dest_file, char *opt);
  char *support_opt; // Supported options for this unary_command
} binary_command;

enum arg_type
  {
    CMD,
    TAR_FILE,
    REG_FILE,
    OPTION,
    ERROR
  };

struct tar_file
{
  char *tar_name;
  char *filename;
};

struct arg
{
  enum arg_type type;
  
  union
  {
    char *value;
    struct tar_file tf;
  };
  
};

/* Handle all the arguments of a unary_command wherever the options are and
   in whatever ordre the arguments (inside/outside tar) are */
int handle_unary_command (unary_command cmd, int argc, char **argv);

int handle_binary_command (binary_command cmd, int argc, char **argv);


char *check_options (int argc, char **argv, char *optstring);
void invalid_options (char *cmd_name);

struct arg *tokenize_args (int argc, char **argv);
void free_tokens (struct arg *tokens, int tokens_size);
char **tokens_to_argv (struct arg *tokens, int tokens_size);

void init_arg_info (arg_info *info, struct arg *tokens, int tokens_size);
int get_nb_valid_file (arg_info *info, char *options);
bool no_arg (arg_info *info);


#endif
