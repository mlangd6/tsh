#ifndef PARSE_LINE_H
#define PARSE_LINE_H

typedef enum {
  ARG,
  REDIR
} token_type;

typedef union {
  char *arg;
  redir_type *red;
} token_value;

typedef struct {
  token_value val;
  token_type type;
} token;


int count_words(const char *str);
char **split(char *user_input, int *is_special);



#endif
