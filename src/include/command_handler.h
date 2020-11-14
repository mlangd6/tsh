#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

typedef struct command {
  char *cmd_name;
  int *(*in_tar_func) (char *, char *, char *);
  char *supported_opt;
} command;

int handle(command c);

#endif
