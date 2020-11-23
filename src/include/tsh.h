#ifndef TSH_H
#define TSH_H

#define PROMPT "$ "
#define CMD_NOT_FOUND " : command not found\n"
#define CMD_NOT_FOUND_SIZE 22

#define NB_TAR_CMD 2
#define NB_TSH_FUNC 3
#define TAR_CMD 1
#define TSH_FUNC 2
#define NB_REDIR 5

typedef struct {
  char *identifier;
  void (*before)(char *);
  void (*after)(char *);
} redir_type;



int special_command(char *s);


#endif
