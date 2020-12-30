/**
 * @file command_handler.h
 * Tar command handler
 */

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <stdbool.h>
#include <stddef.h>

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
  bool has_error; // 1 if a file argument is an error (e.g doesn't exists)
  
  size_t options_size; // Number of options
  char **options; // Array containing all the options + unary_command name
} arg_info ;

typedef struct binary_command
{
  char *name; // Name of command
  int (*tar_to_tar)(char *src_tar, char *src_file, char *dest_tar, char *dest_file, char *opt);
  int (*extern_to_tar)(char *src_file, char *dest_tar, char *dest_file, char *opt);
  int (*tar_to_extern)(char *src_tar, char *src_file, char *dest_file, char *opt);
  char *support_opt; // Supported options for this binary_command
} binary_command;

enum arg_type
  {
    CMD,
    TAR_FILE,
    REG_FILE,
    OPTION
  };

struct tar_path
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
    struct tar_path tf;
  };

};

/**
 * Handles a unary command (a command such as `cat, ls, rmdir, mkdir, rm`)
 *
 * There is no limitation on the places of the options and the order of the arguments (inside/outside a tar).
 **/
int handle_unary_command (unary_command cmd, int argc, char **argv);

/**
 * Handles a binary command (such as `mv` and `cp`)
 *
 * There is no limitation on the places of the options and the order of the arguments (inside/outside a tar).
 * In all cases, at least two arguments are needed.
 */
int handle_binary_command (binary_command cmd, int argc, char **argv);

/**
 * Gets a string of all detected and valid options in `argv`.
 *
 * All non-options are moved to the end due to `getopt`.
 */
char *check_options (int argc, char **argv, char *optstring);

/** Prints an error message caused by an invalid option */
void invalid_options (char *cmd_name);

/**
 * Gets a `struct arg` array of size `argc` by scanning `argv`.
 */
struct arg *tokenize_args (int *argc, char **argv, arg_info *info);

/**
 * Free a `struct arg` array of size `tokens_size`
 */
void free_tokens (struct arg *tokens, int tokens_size);

/**
 * Calls `execvp` on tokens
 */
int execvp_tokens (char *cmd_name, struct arg *tokens, int tokens_size);

/** Init a `struct arg_info` given `tokens` */
void init_arg_info (arg_info *info, struct arg *tokens, int tokens_size);

/** Gets the number of valid file. */
int get_nb_valid_file (arg_info *info, char *options);

/** Checks if there is no argument at all */
bool no_arg (arg_info *info);


#endif
