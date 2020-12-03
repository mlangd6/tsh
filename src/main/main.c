#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <linux/limits.h>

#include "tsh.h"
#include "path_lib.h"
#include "tar.h"
#include "errors.h"
#include "parse_line.h"


static int ret_value;

static int cd(char **argv, int argc);
static int exit_tsh(char **argv, int argc);
static int pwd(char **argv, int argc);

static void init_tsh();

static int launch_tsh_func(char **argv, int argc);


char tsh_dir[PATH_MAX];



static void init_tsh()
{
  char cwd[PATH_MAX];
  getcwd(cwd, PATH_MAX);
  setenv("PWD", cwd, 1); // In case PWD has not the right value at the begining
  char *home = getenv("HOME");
  strcpy(tsh_dir, home);
  strcat(tsh_dir, "/.tsh");
  ret_value = EXIT_SUCCESS;
}

static int launch_tsh_func(char **argv, int argc)
{
  if (strcmp(argv[0], "cd") == 0)
    {
      return cd(argv, argc);
    }
  else if (strcmp(argv[0], "exit") == 0)
    {
      exit_tsh(argv, argc);
    }
  else if (strcmp(argv[0], "pwd") == 0)
    {
      return pwd(argv, argc);
    }
  return EXIT_FAILURE;
}

static int pwd(char **argv, int argc)
{
  char *pwd = getenv("PWD");
  return
    (write(STDOUT_FILENO, pwd, strlen(pwd)) < 0 ||
     write(STDOUT_FILENO, "\n", 2) < 0) ?
    EXIT_FAILURE :
    EXIT_SUCCESS;
}

static int exit_tsh(char **argv, int argc)
{
  for (int i = 1; i < argc; i++)
    {
      free(argv[i]);
    }
  free(argv);
  exit(ret_value);
}

static int cd(char **argv, int argc)
{
  char *pwd = getenv("PWD");
  if (argc == 1)
  {
    char *home = getenv("HOME");

    if (home == NULL)
    {
      char *error = "tsh: cd: HOME not set\n";
      write(STDERR_FILENO, error, strlen(error) + 1);
      return EXIT_FAILURE;
    }

    if (chdir(home) != 0)
    {
      return EXIT_FAILURE;
    }
    char *pwd = getenv("PWD");
    setenv("OLDPWD", pwd, 1);
    setenv("PWD", home, 1);
    return EXIT_SUCCESS;
  }
  else if (argc == 2)
  {
    if (argv[1][0] == '-') // Option
    {
      if (argv[1][1] != '\0') // Not supported option
      {
        char *err = "tsh: cd: - est la seul option supporté\n";
        write(STDERR_FILENO, err, strlen(err) + 1);
        return EXIT_FAILURE;
      }
      else // Supported option
      {
        char *oldpwd = getenv("OLDPWD");
        if (oldpwd == NULL)
        {
          char *err = "tsh: cd: \" OLDPWD \" non défini\n";
          write(STDERR_FILENO, err, strlen(err) + 1);
          return EXIT_FAILURE;
        }
        else
        {
          char tmp[PATH_MAX];
          strcpy(tmp, oldpwd);
          setenv("OLDPWD", pwd, 1);
          setenv("PWD", tmp, 1);
          write(STDOUT_FILENO, oldpwd, strlen(oldpwd));
          write(STDOUT_FILENO, "\n", 2);
          chdir(tmp);
        }
      }
    }
    else // Not an option
    {
      char arg[PATH_MAX];
      char path[PATH_MAX];
      if (argv[1][0] != '/')
        sprintf(arg, "%s/%s", getenv("PWD"), argv[1]);
      else
        strcpy(arg, argv[1]);
      int arg_len = strlen(arg);
      if (arg_len + 1 >= PATH_MAX) //too long
      {
        return EXIT_FAILURE;
      }

      if (arg[arg_len-1] != '/')
      strcat(arg, "/");

      if (!reduce_abs_path(arg, path))
      {
        error_cmd("tsh: cd", arg);
        return EXIT_FAILURE;
      }

      char *in_tar = split_tar_abs_path(path);

      if (!in_tar) //No TAR implied
      {
        if (chdir(path) != 0)
        {
          error_cmd("tsh: cd", argv[1]);
          return EXIT_FAILURE;
        }

        setenv("OLDPWD", pwd, 1);
        setenv("PWD", path, 1);
      }
      else //TAR implied
      {
        if (*in_tar != '\0') // path inside tar file
        {
          if (tar_access(path, in_tar, X_OK) != -1) // check execution permission
          {
            int in_tar_len = strlen(in_tar);
            in_tar[in_tar_len-1] = '\0';
            in_tar[-1] = '/';
            setenv("OLDPWD", pwd, 1);
            setenv("PWD", path, 1);
            in_tar[-1] = '\0';
          }
          else
          {
            in_tar[-1] = '/';
            perror(argv[1]);
            return EXIT_FAILURE;
          }
        }
        else // going into the root of the TAR
        {
          setenv("OLDPWD", pwd, 1);
          setenv("PWD", path, 1);
        }

        char *before_tar = strrchr(path, '/'); // Not NULL
        *before_tar = '\0';
        chdir(path);
      }
    }
    return EXIT_SUCCESS;
  }
  else
  {
    char *err = "tsh: cd: trop d'arguments\n";
    write(STDERR_FILENO, err, strlen(err));
    return EXIT_FAILURE;
  }
}


int main (int argc, char *argv[])
{
  init_tsh ();

  char *buf;
  token **tokens;
  char **args;
  int nb_tokens;
  pid_t cpid;
  int  is_special, wstatus;

  while ((buf = readline(PROMPT)))
  {
    if (!count_words(buf))
      continue;

    add_history(buf);

    tokens = tokenize(buf, &nb_tokens);
    args = malloc((nb_tokens + 1) * sizeof(char *));
    nb_tokens = exec_tokens(tokens, nb_tokens, args);
    if (nb_tokens == 0) // TODO: Les redirections devront être annulés
      continue;
    is_special = special_command(args[0]);
    if (is_special == TSH_FUNC)
    {
      ret_value = launch_tsh_func(args, nb_tokens);
    }
    else
    {
      cpid = fork();
      switch (cpid)
      {
        case -1:
        perror ("tsh: fork");
        exit (EXIT_FAILURE);

        case 0: // child
        if (is_special == TAR_CMD)
        {
          char cmd_exec[PATH_MAX];
          strcpy(cmd_exec, tsh_dir);
          strcat(cmd_exec, "/bin/");
          strcat(cmd_exec, args[0]);
          ret_value = execv(cmd_exec, args);
        }
        else
        {
          ret_value = execvp(args[0], args);
        }

        if (errno == ENOENT)
        {
          int size = strlen(args[0]) + CMD_NOT_FOUND_SIZE;
          char error_msg[size];
          strcpy(error_msg, args[0]);
          strcat(error_msg, CMD_NOT_FOUND);
          write(STDOUT_FILENO, error_msg, size);
        }

        exit(EXIT_FAILURE);

        default: // father
        wait(&wstatus);
        ret_value = WEXITSTATUS(wstatus);
      }

    }
    free(args);
    free(buf);
  }

  return 0;
}
