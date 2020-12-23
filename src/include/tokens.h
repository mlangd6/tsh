#include "redirection.h"
#include "list.h"

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



#endif
