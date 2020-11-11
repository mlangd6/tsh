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
#include "errors.h"

#define PROMPT "$ "
#define CMD_NOT_FOUND " : command not found\n"
#define CMD_NOT_FOUND_SIZE 22
#define NB_TAR_CMD 2
#define NB_TSH_FUNC 3
#define TAR_CMD 1
#define TSH_FUNC 2
#define MAX_TSH_FUNC_SIZE 5
#define SET_TSH_FUNC 1
#define NO_SET_TSH_FUNC 0

static int ret_value;

static int cd(char **argv, int argc);
static int exit_tsh(char **argv, int argc);
static int pwd(char **argv, int argc);

static int count_words(char *s);
static char **split(char *s, int *spec);
static void init_tsh();

static int special_command(char *s, char **tab, int tab_l, int set_tsh_func);
static int launch_tsh_func(char **argv, int argc);
static int count_argc(char **argv);
static char *remove_excessive_spaces_string(char *s);


char tsh_dir[PATH_MAX];
char *tar_cmds[NB_TAR_CMD] = {"cat", "ls"};
char *tsh_funcs[NB_TSH_FUNC] = {"cd", "exit", "pwd"};

char twd[PATH_MAX];
char tsh_func[MAX_TSH_FUNC_SIZE];

static int count_words(char *s) {
  int res = 1;
  char *chr = s;
  while((chr = strchr(chr, ' ')) != NULL && chr[1] != '\0' && res++)
  {
    chr++;
  }
  return res;
}

static char *remove_excessive_spaces_string(char *s) {
  int spaces = 0;
  char *chr = s;
  while(chr[0] == ' ') chr++;
  char *copy = malloc(strlen(chr));
  int i = 0, cmp = 0;
  for(i = 0; i < strlen(chr); i++){
    if(chr[i] == ' ' && spaces > 0)spaces++;
    else if(chr[i] == ' ' && spaces == 0){
      copy[cmp++] = chr[i];
      spaces = 1;
    }
    else {
      copy[cmp++] = chr[i];
      spaces = 0;
    }
  }
  if(copy[cmp-1] == ' ') copy[cmp-1] = '\0';
  else copy[cmp] = '\0';
  free(s);
  return copy;
}

static char **split(char *s, int *spec) {
  int nb_tokens = count_words(s);

  char delim[] = " ";
  char **res = malloc((nb_tokens+1) * sizeof(char *));

  res[0] = strtok(s, delim);


  *spec = (special_command(res[0], tar_cmds, NB_TAR_CMD, NO_SET_TSH_FUNC)) ? 1 :
    (special_command(res[0], tsh_funcs, NB_TSH_FUNC, SET_TSH_FUNC)) ? 2 : 0;

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
  ret_value = EXIT_SUCCESS;
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

static int launch_tsh_func(char **argv, int argc) {
  if (strcmp(argv[0], "cd") == 0) {
    return cd(argv, argc);
  }
  else if (strcmp(argv[0], "exit") == 0) {
    exit_tsh(argv, argc);
  }
  else if (strcmp(argv[0], "pwd") == 0) {
    return pwd(argv, argc);
  }
  return EXIT_FAILURE;
}

static int count_argc(char **argv) {
  int i = 0;
  while (argv[i++] != NULL) ;
  return i-1;
}

static int pwd(char **argv, int argc) {
  return (
    write(STDOUT_FILENO, twd, strlen(twd)) < 0 ||
    write(STDOUT_FILENO, "\n", 2) < 0
  ) ? EXIT_FAILURE : EXIT_SUCCESS;
}

static int exit_tsh(char **argv, int argc) {
  for (int i = 1; i < argc; i++)
  {
    free(argv[i]);
  }
  free(argv);
  exit(ret_value);
}

static int cd(char **argv, int argc) {
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
    setenv("OLDPWD", twd, 1);
    memcpy(twd, home, strlen(home) + 1);
    return EXIT_SUCCESS;
  }
  else if (argc == 2) {
    if (argv[1][0] == '-') { // Option
      if (argv[1][1] != '\0') { // Not supported option
        char *err = "tsh: cd: - est la seul option supporté\n";
        write(STDERR_FILENO, err, strlen(err) + 1);
        return EXIT_FAILURE;
      }
      else { // Supported option
        free(argv[1]);
        char *oldpwd = getenv("OLDPWD");
        if (oldpwd == NULL) {
          char *err = "tsh: cd: \" OLDPWD \" non défini\n";
          write(STDERR_FILENO, err, strlen(err) + 1);
          return EXIT_FAILURE;
        }
        else {
          argv[1] = malloc(PATH_MAX);
          memcpy(argv[1], oldpwd, strlen(oldpwd) + 1);
          write(STDOUT_FILENO, oldpwd, strlen(oldpwd));
          write(STDOUT_FILENO, "\n", 2);
        }
      }
    }
    else // Not an option
    {
      char *in_tar = split_tar_abs_path(argv[1]);
      int is_tar_dir = is_tar(argv[1]);
      if (*in_tar == '\0' && is_tar_dir != 1) { // Not tar implied
        if (chdir(argv[1]) != 0) {
          error_cmd("tsh: cd", argv[1]);
          return EXIT_FAILURE;
        }
        setenv("PWD", argv[1], 1);
        setenv("OLDPWD", twd, 1);
        memcpy(twd, argv[1], strlen(argv[1]) + 1);
      }
      else { // in tar => path is valid (but not necessarily in_tar)
        if (*in_tar != '\0') { // path inside tar file
          int in_tar_len = strlen(in_tar);
          memcpy(in_tar + in_tar_len, "/", 2);
          if (tar_access(argv[1], in_tar, X_OK) != -1) {
            in_tar[-1] = '/';
            in_tar[in_tar_len] = '\0';
            setenv("PWD", argv[1], 1);
            setenv("OLDPWD", twd, 1);
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
          setenv("OLDPWD", twd, 1);
          memcpy(twd, argv[1], strlen(argv[1]) + 1);
        }
        char *bef_tar = strrchr(argv[1], '/'); // Not NULL
        *bef_tar = '\0';
        chdir(argv[1]);
      }
    }
    return EXIT_SUCCESS;
  }
  else {
    char *err = "tsh: cd: trop d'arguments\n";
    write(STDERR_FILENO, err, strlen(err));
    return EXIT_FAILURE;
  }
}

int main(int argc, char *argv[]){
  init_tsh();
  char *buf;
  int spec;
  while((buf = readline(PROMPT))) {
    buf = remove_excessive_spaces_string(buf);
    size_t buf_size = strlen(buf);
    if(buf_size == 0)
      continue;
    if (buf_size > 0) {
      add_history(buf);
    }
    char **tokens = split(buf, &spec);
    if (spec == TSH_FUNC) {
      ret_value = launch_tsh_func(tokens, count_argc(tokens));
    }
    else {
      int p = fork(), status;
      switch (p) {
        case -1:
          perror("fork");
          exit(EXIT_FAILURE);
          case 0: // child
          if (spec == TAR_CMD) {
            char cmd_exec[PATH_MAX];
            strcpy(cmd_exec, tsh_dir);
            strcat(cmd_exec, "/bin/");
            strcat(cmd_exec, tokens[0]);
            ret_value = execv(cmd_exec, tokens);
          }
          else {
            ret_value = execvp(tokens[0], tokens);
          }
          if (errno == ENOENT) {
            int size = strlen(tokens[0]) + CMD_NOT_FOUND_SIZE;
            char error_msg[size];
            strcpy(error_msg, tokens[0]);
            strcat(error_msg, CMD_NOT_FOUND);
            write(STDERR_FILENO, error_msg, size);
          }
          exit(EXIT_FAILURE);
          default: // parent
            wait(&status);
            ret_value = WEXITSTATUS(status);
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
