#include "redirection.h"
#include "list.h"
#include "array.h"

#ifndef PARSE_LINE_H
#define PARSE_LINE_H

typedef enum {
  ARG,
  REDIR,
  PIPE
} token_type;

typedef union {
  char *arg;
  redir_type red;
} token_value;

typedef struct {
  token_value val;
  token_type type;
} token;

int count_words(const char *str);
list *tokenize(char *user_input);
int exec_tokens(token **tokens, int nb_el, char **argv);
int exec_line(char *line);
int exec_cmd_array(array *cmd);
char **cmd_array_to_argv(array *cmd_arr);
void remove_all_redir(array *cmd);
int exec_red_array(array *cmd);


#endif
