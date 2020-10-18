#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <linux/limits.h>
#include "path_lib.h"
#include "tar.h"

#define PROMPT "$ "
#define CMD_NOT_FOUND " : command not found\n"
#define CMD_NOT_FOUND_SIZE 22
#define NB_TAR_CMD 1

char tsh_dir[PATH_MAX];
char *tar_cmds[NB_TAR_CMD] = {"cat"};
char act_path[PATH_MAX];

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
    if (res[i][0] != '-') { // Not an option
      if (res[i][0] != '/') { // Relative path
        char *tmp = malloc(PATH_MAX);
        strcpy(tmp, act_path);
        tmp[strlen(tmp)] = '/';
        strcat(tmp, res[i]);
        res[i] = tmp;
      }
      else { // Absolute path
        char *tmp = malloc(PATH_MAX);
        strcpy(tmp, res[i]);
        res[i] = tmp;
      }
      reduce_abs_path(res[i]);
    }
  }
  res[nb_tokens] = NULL;
  return res;
}

static void init_tsh() {
  getcwd(act_path, PATH_MAX);
  char *home = getenv("HOME");
  strcpy(tsh_dir, home);
  strcat(tsh_dir, "/.tsh");
}

static int is_tar_command(char *s) {
  for (int i = 0; i < NB_TAR_CMD; i++) {
    if (strcmp(s, tar_cmds[i]) == 0) {
      return 1;
    }
  }
  return 0;
}

int main(int argc, char *argv[]){
  init_tsh();
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
        if (is_tar_command(tokens[0])) {
          char cmd_exec[PATH_MAX];
          strcpy(cmd_exec, tsh_dir);
          strcat(cmd_exec, "/bin/");
          strcat(cmd_exec, tokens[0]);
          execv(cmd_exec, tokens);
        }
        else {
          execvp(tokens[0], tokens);
        }
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
