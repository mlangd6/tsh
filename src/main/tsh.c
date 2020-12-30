#include <string.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "redirection.h"
#include "path_lib.h"
#include "errors.h"
#include "tar.h"
#include "tsh.h"
#include "utils.h"

static int ret_value;
static int cd(char **argv, int argc);
static int pwd(char **argv, int argc);
static int exit_tsh(char **argv, int argc);

const char *tar_cmds[NB_TAR_CMD] = {"cat", "ls", "rm", "mkdir", "rmdir", "mv", "cp"};
const char *tsh_funcs[NB_TSH_FUNC] = {"cd", "exit", "pwd"};

char tsh_dir[PATH_MAX];

void set_ret_value(int ret)
{
  ret_value = ret;
}

int special_command(char *s)
{
  int max = (NB_TAR_CMD > NB_TSH_FUNC) ? NB_TAR_CMD : NB_TSH_FUNC;
  for (int i = 0; i < max; i++)
  {
    if (i < NB_TAR_CMD && strcmp(s, tar_cmds[i]) == 0)
    {
      return TAR_CMD;
    }
    if (i < NB_TSH_FUNC && strcmp(s, tsh_funcs[i]) == 0)
    {
      return TSH_FUNC;
    }
  }
  return 0;
}

void init_tsh_dir()
{
  strcpy(tsh_dir, "/tmp/.tsh");
}

char *get_tsh_dir(char *buf)
{
  strcpy(buf, tsh_dir);
  return buf;
}

int launch_tsh_func(char **argv, int argc)
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
  exit_redirections();
  free(argv[0]);
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
      error (0, "tsh: cd: HOME not set\n");
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
	error (0, "tsh: cd: - est la seule option supportée\n");
        return EXIT_FAILURE;
      }
      else // Supported option
      {
        char *oldpwd = getenv("OLDPWD");
        if (oldpwd == NULL)
        {
	  error (0, "tsh: cd: \" OLDPWD \" non défini\n");
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
        if (strcmp(path, "/") != 0) remove_last_slash(path);
        setenv("OLDPWD", pwd, 1);
        setenv("PWD", path, 1);
      }
      else //TAR implied
      {
        if (*in_tar != '\0') // path inside tar file
        {
          if (tar_access(path, in_tar, X_OK) != -1) // check execution permission
          {
            remove_last_slash(in_tar);
            char buf[PATH_MAX + 1];
            sprintf(buf, "%s/%s", path, in_tar);
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
    error (0, "tsh: cd: trop d'arguments\n");
    return EXIT_FAILURE;
  }
}

char *set_prompt(char *prompt)
{
  char c = getuid() == 0 ? '#': '$';
  sprintf(prompt, "%s%c ", getenv("PWD"), c);
  return prompt;
}
