#include "redirection.h"

#ifndef PARSE_LINE_H
#define PARSE_LINE_H

typedef enum {
  ARG,
  REDIR
} token_type;

typedef union {
  char *arg;
  redir_type red;
} token_value;

typedef struct {
  token_value val;
  token_type type;
} token;

token *char_to_token(char *w);
int count_words(const char *str);
token **tokenize(char *user_input, int *nb_el);
int exec_tokens(token **tokens, int nb_el, char **argv);
char **split(char *user_input, int *is_special);



#endif
