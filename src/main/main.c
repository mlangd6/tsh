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
#define NB_TAR_CMD 2
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
char *tar_cmds[NB_TAR_CMD] = {"cat", "ls"};
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

static int launch_tsh_func(char **argv) {
  if (strcmp(argv[0], "cd") == 0) {
    return cd(argv);
  }
  return 1;
}

static int count_argc(char **argv) {
  int i = 0;
  while (argv[i++] != NULL) ;
  return i-1;
}

static int cd(char **argv) {
  char *pre_error = "tsh: cd: ";
  int argc = count_argc(argv);
  if (argc == 1) {
    char *home = getenv("HOME");
    if (home == NULL) {
      char *error = "tsh: cd: HOME not set\n";
      write(STDERR_FILENO, error, strlen(error) + 1);
      return EXIT_FAILURE;
    }
    if (chdir(home) != 0) {
      return EXIT_FAILURE;
    }
    setenv("PWD", home, 1);
    memcpy(twd, home, strlen(home) + 1);
    return EXIT_SUCCESS;
  }
  else if (argc == 2) {
    char *in_tar = split_tar_abs_path(argv[1]);
    int is_tar_dir = is_tar(argv[1]);
    if (*in_tar == '\0' && is_tar_dir != 1) { // Not tar implied
      if (chdir(argv[1]) != 0) {
        write(STDERR_FILENO, pre_error, strlen(pre_error));
        perror(argv[1]);
        return EXIT_FAILURE;
      }
      setenv("PWD", argv[1], 1);
      memcpy(twd, argv[1], strlen(argv[1]) + 1);
    }
    else { // in tar => path is valid (but not necessarily in_tar)
      if (*in_tar != '\0') { // path inside tar file
        int in_tar_len = strlen(in_tar);
        memcpy(in_tar + in_tar_len, "/", 2);
        if (tar_access(argv[1], in_tar, F_OK) != -1) {
          in_tar[-1] = '/';
          in_tar[in_tar_len] = '\0';
          setenv("PWD", argv[1], 1);
          memcpy(twd, argv[1], strlen(argv[1]) + 1);
          in_tar[-1] = '\0';
        }
        else {
          in_tar[-1] = '/';
          perror(argv[1]);
          return EXIT_FAILURE;
        }
      }
      else {
        setenv("PWD", argv[1], 1);
        memcpy(twd, argv[1], strlen(argv[1]) + 1);
      }
      char *bef_tar = strrchr(argv[1], '/'); // Not NULL
      *bef_tar = '\0';
      chdir(argv[1]);
    }
  }
  return EXIT_SUCCESS;
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
    if (spec == TSH_FUNC) {
      launch_tsh_func(tokens);
    }
    else {
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
          else {
            execvp(tokens[0], tokens);
          }
          if (errno == ENOENT) {
            int size = strlen(tokens[0]) + CMD_NOT_FOUND_SIZE;
            char error_msg[size];
            strcpy(error_msg, tokens[0]);
            strcat(error_msg, CMD_NOT_FOUND);
            write(STDERR_FILENO, error_msg, size);
          }
          exit(EXIT_FAILURE);
          default: // father
          wait(&w);
        }
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
