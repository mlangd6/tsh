#ifndef TSH_H
#define TSH_H

#define PROMPT "$ "
#define CMD_NOT_FOUND " : command not found\n"
#define CMD_NOT_FOUND_SIZE 22

#define NB_TAR_CMD 2
#define NB_TSH_FUNC 3
#define TAR_CMD 1
#define TSH_FUNC 2

typedef struct {
  void (*before)(char *);
  void (*after)(char *);
} redir_type;

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


int special_command(char *s);


#endif
