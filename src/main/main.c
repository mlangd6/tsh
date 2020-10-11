#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/wait.h>
#include "tar.h"

#define PROMPT "$ "

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
  printf("Hello World!\n");
  return 0;
}
