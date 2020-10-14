#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "tar.h"

#define PROMPT "$ "
#define CMD_NOT_FOUND " : command not found\n"
#define CMD_NOT_FOUND_SIZE 22

static int count_words(char *s) {
  int res = 1;
  char *prev = s;
  char *next;
  while((next = strchr(prev, ' ')) != NULL) {
    if (next != prev+1 && *(next+1) != '\0' && res++) ;
    prev = next+1;
  }
  return res;
}

static char **split(char *s) {
  int nb_tokens = count_words(s);
  char delim[] = " ";
  char **res = malloc((nb_tokens+1) * sizeof(char *));
  res[0] = strtok(s, delim);
  for (int i = 1; i < nb_tokens; i++) {
    res[i] = strtok(NULL, delim);
  }
  res[nb_tokens] = NULL;
  return res;
}

int main(int argc, char *argv[]){
  char *buf;
  while((buf = readline(PROMPT))) {
    if (strlen(buf) > 0) {
      add_history(buf);
    }
    char **tokens = split(buf);
    int p = fork(), w;
    switch (p) {
      case -1:
        perror("fork");
        exit(EXIT_FAILURE);
      case 0: //son
        execvp(tokens[0], tokens);
        if (errno == ENOENT) {
          int size = strlen(tokens[0]) + CMD_NOT_FOUND_SIZE;
          char error_msg[size];
          strcpy(error_msg, tokens[0]);
          strcat(error_msg, CMD_NOT_FOUND);
          write(STDOUT_FILENO, error_msg, size);
        }
        exit(EXIT_FAILURE);
      default: // father
        wait(&w);

    }


  }
  return 0;
}
