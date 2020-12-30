/**
 * @file tokens.h
 * Manipulation of user_input and tokens of an input line
 */

#include "redirection.h"
#include "list.h"
#include "array.h"

#include <stdbool.h>

#ifndef PARSE_LINE_H
#define PARSE_LINE_H

/**
 * The differents type of [arguments/tokens] in an input line.
 */
typedef enum {
  ARG, /**< A string that is not a redirections or a pipe. */
  REDIR, /**< A redirection. */
  PIPE /**< A pipe. */
} token_type;

/**
 * The content of an input line or the type of the redirection.
 */
typedef union {
  char *arg; /**< the content of the argument. */
  redir_type red; /**< The type of the redirection */
} token_value;

/**
 * The type of the tokens and its content
 */
typedef struct {
  token_value val; /**< The [value/content] of the token */
  token_type type; /**< The type of the token */
} token;

/**
 * Count the number of words in a string.
 * @param str the string.
 * @return the number of words.
 */
int count_words(const char *str);

/**
 * Split a user_input into a list of array of tokens.
 * each array represents a command and its arguments.
 * The list represents all the commands.
 * There will be n+1 array in the list where n represent the number of '|' char.
 * Each array will end by a PIPE token.
 * @param str The string.
 * @return The list of array of tokens.
 */
list *tokenize(char *user_input);

/**
 * Execute a line with its pipe, redirections and command.
 * It will create the necessary number of process.
 * @param line The line to execute.
 * @return the result of the launch commands.
 */
int exec_line(char *line);

/**
 * Exec an array of tokens, the array must ends by a PIPE token per convention.
 * Only represents one command (no pipe in it).
 * It will launch the redirections, then the command.
 * @param cmd The array of token.
 * @return the result of the command.
 */
int exec_cmd_array(array *cmd);

/**
 * Create an vector of string from an array of tokens.
 * The array should not have any redirection tokens in it.
 * @param cmd_arr The array.
 * @return the vector of string corresponding to cmd_arr.
 */
char **cmd_array_to_argv(array *cmd_arr);

/**
 * Remove all the redirections tokens from an array of tokens.
 * @param cmd The array.
 */
void remove_all_redir_tokens(array *cmd);

/**
 * Launch redirections form an array of tokens.
 * @param cmd The array of tokens.
 * @return 0 if all the redirections succeed, else -1.
 */
int exec_red_array(array *cmd);

/**
 * Check if a list of array of tokens is well formatted.
 * This is true if between every pipe there is something and
 * a redirections is always followed by an argument.
 * @param cmds the list of array of tokens.
 * @return true if the list the list if well formatted esle false.
 */
bool parse_tokens(list *cmds);

/**
 * Free a list of array tokens.
 * @param tokens the list of array of tokens.
 */
void free_tokens_list(list *tokens);

#endif
