#include <stdio.h>
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
#define NB_TSH_FUNC 1
#define TAR_CMD 1
#define TSH_FUNC 2
#define MAX_TSH_FUNC_SIZE 3
#define SET_TSH_FUNC 1
#define NO_SET_TSH_FUNC 0

static int count_words(char *s);
static char **split(char *s, int *is_tar_cmd);
static void init_tsh();
static int special_command(char *s, char **tab, int tab_l, int set_tsh_func);
static int launch_tsh_func(char **argv);
static int cd(char **argv);
static int count_argc(char **argv);

char tsh_dir[PATH_MAX];
char *tar_cmds[NB_TAR_CMD] = {"cat"};
char *tsh_funcs[NB_TSH_FUNC] = {"cd"};
char twd[PATH_MAX];
char tsh_func[MAX_TSH_FUNC_SIZE];

static int count_words(char *s) {
  int res = 1;
  char *chr = s;
  while((chr = strchr(chr, ' ')) != NULL && chr[1] != '\0' && res++) {
    chr++;
  }
  return res;
}

static char **split(char *s, int *spec) {
  int nb_tokens = count_words(s);
  char delim[] = " ";
  char **res = malloc((nb_tokens+1) * sizeof(char *));
  res[0] = strtok(s, delim);
  *spec = (special_command(res[0], tar_cmds, NB_TAR_CMD, NO_SET_TSH_FUNC)) ? TAR_CMD :
    (special_command(res[0], tsh_funcs, NB_TSH_FUNC, SET_TSH_FUNC)) ? TSH_FUNC : 0;
  for (int i = 1; i < nb_tokens; i++) {
    res[i] = strtok(NULL, delim);
    if (*spec) {
      if (res[i][0] != '-') { // Not an option
        if (res[i][0] != '/') { // Relative path
          char *tmp = malloc(PATH_MAX);
          strcpy(tmp, twd);
          strcat(tmp, "/");
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
      else {
        char *tmp = malloc(sizeof(res[i]) + 1);
        strcpy(tmp, res[i]);
        res[i] = tmp;
      }
    }
  }
  res[nb_tokens] = NULL;
  return res;
}

static void init_tsh() {
  getcwd(twd, PATH_MAX);
  char *home = getenv("HOME");
  strcpy(tsh_dir, home);
  strcat(tsh_dir, "/.tsh");
}

static int special_command(char *s, char **tab, int tab_l, int set_tsh_func) {
  for (int i = 0; i < tab_l; i++) {
    if (strcmp(s, tab[i]) == 0) {
      if (set_tsh_func) {
        memcpy(tsh_func, tab[i], strlen(tab[i]) + 1);
      }
      return 1;
    }
  }
  return 0;
}

int main(int argc, char *argv[]){
  init_tsh();
  char *buf;
  int spec;
  while((buf = readline(PROMPT))) {
    if (strlen(buf) > 0) {
      add_history(buf);
    }
    char **tokens = split(buf, &spec);
    int p = fork(), w;
    switch (p) {
      case -1:
        perror("fork");
        exit(EXIT_FAILURE);
      case 0: //son
        if (spec == TAR_CMD) {
          char cmd_exec[PATH_MAX];
          strcpy(cmd_exec, tsh_dir);
          strcat(cmd_exec, "/bin/");
          strcat(cmd_exec, tokens[0]);
          execv(cmd_exec, tokens);
        }
        else if (spec == TSH_FUNC) {
          launch_tsh_func(tokens);
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
    if (spec) {
      for (int i = 1; tokens[i] != NULL; i++) {
        free(tokens[i]);
      }
    }
    free(tokens);
    free(buf);
  }
  return 0;
}
