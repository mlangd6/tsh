#include <linux/limits.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <stdio.h>

#include "errors.h"
#include "path_lib.h"
#include "tar.h"
#include "command_handler.h"

static void invalid_options(char *cmd_name);
static char **parse_args(int *argc, char **argv, arg_info *info);
static int handle_with_pwd(command *cmd, char **argv, int not_tar_opt, char *tar_opt);
static int handle_one_it_tar(command *cmd, char *arg, char *path, char *in_tar,
  char *tar_opt, int not_tar_opt, int nb_valid_arg, int *rest);
static int handle_one_it_out(command *cmd, int nb_valid_arg, char **argv, int argc, int *rest);
static void print_arg_before(command *cmd, int nb_valid_arg, char *argv);
static void print_arg_after(command *cmd, int *rest);
static int check_options(int argc, char **argv, command *cmd, char *tar_opt, int *tar_opt_c);


int handle(command cmd, int argc, char **argv) {
  arg_info info = {
    0,
    0,
    0,
    0,
    calloc(argc+1, sizeof(char *))
  };
  info.options[info.opt_c++] = cmd.name;
  argv = parse_args(&argc, argv, &info);

  if (info.nb_in_tar == 0 && (!cmd.twd_arg || info.nb_out > 0)) // No tar involved
  {
    if (info.nb_out > 0 || !info.err_arg) // No error
      execvp(cmd.name, argv);
    else
      return EXIT_FAILURE;
  }
  opterr = 0;
  int ret = EXIT_SUCCESS;
  int sup_opt_len = strlen(cmd.support_opt);
  char *tar_opt = malloc(sup_opt_len + 1);
  memset(tar_opt, '\0', sup_opt_len + 1);
  int tar_opt_c = 0;
  int not_tar_opt = check_options(argc, argv, &cmd, tar_opt, &tar_opt_c);
  if (info.nb_in_tar == 0 && info.nb_out == 0) // No arguments (cmd.twd_arg == 1)
  {
    if (info.err_arg)
      return EXIT_FAILURE;
    return handle_with_pwd(&cmd, argv, not_tar_opt, tar_opt);
  }
  if (not_tar_opt)
    invalid_options(cmd.name);
  int rest = info.nb_out + ((not_tar_opt) ? 0: info.nb_in_tar);
  int nb_valid_arg = rest;
  char *in_tar;
  char path[PATH_MAX];
  for (size_t i = 1; i < argc; i++)
  {
    if (*argv[i] == '-')
      continue;
    if (reduce_abs_path(argv[i], path) == NULL)
    {
      ret = EXIT_FAILURE;
      continue;
    }
    in_tar = split_tar_abs_path(path);
    if (in_tar != NULL) // Tar involved
    {
      if (handle_one_it_tar(&cmd, argv[i], path, in_tar, tar_opt, not_tar_opt, nb_valid_arg, &rest) == EXIT_FAILURE)
        ret = EXIT_FAILURE;
    }
    else
    {
      info.options[info.opt_c] = path;
      if (handle_one_it_out(&cmd, nb_valid_arg, info.options, info.opt_c, &rest) == EXIT_FAILURE)
        ret = EXIT_FAILURE;
      info.options[info.opt_c] = NULL;
    }

  }
  free(info.options);
  return ret;
}

static char **parse_args(int *argc, char **argv, arg_info *info)
{
  char **new_argv = calloc(*argc+1, sizeof(char *));
  new_argv[0] = argv[0];
  int new_argc = 1;
  char path[PATH_MAX];
  for (size_t i = 1; i < *argc; i++)
  {
    memset(path, '\0', PATH_MAX);
    if (*argv[i] == '-' && ++(info -> opt_c))
    {
      new_argv[new_argc++] = argv[i];
      info -> options[info -> opt_c-1] = argv[i];
    }
    else
    {
      char *tmp = reduce_abs_path(argv[i], path);
      if (!tmp)
      {
        info -> err_arg = 1;
        error_cmd(argv[0], argv[i]);
        continue;
      }
      if (split_tar_abs_path(path) != NULL)
        info -> nb_in_tar++;
      else
        info -> nb_out++;
      new_argv[new_argc++] = argv[i];
    }
  }
  new_argv[new_argc] = NULL;
  info -> options[info -> opt_c] = NULL;
  info -> options[info -> opt_c+1] = NULL;
  *argc = new_argc;
  return new_argv;
}

static void invalid_options(char *cmd_name)
{
  write(STDERR_FILENO, cmd_name, strlen(cmd_name));
  write(STDERR_FILENO, ": invalid option -- '", 21);
  write(STDERR_FILENO, &optopt, 1);
  write(STDERR_FILENO, "' with tarball, skipping files inside tarball\n", 46);
}

static int handle_with_pwd(command *cmd, char **argv, int not_tar_opt, char *tar_opt)
{
  char *tmp = getenv("PWD");
  char twd[PATH_MAX];
  memcpy(twd, tmp, strlen(tmp) + 1);
  char *in_tar = split_tar_abs_path(twd);
  if (in_tar != NULL)
  {
    if (not_tar_opt)
    {
      invalid_options(cmd -> name);
      return EXIT_FAILURE;
    }
    return cmd -> in_tar_func(twd, in_tar, tar_opt);
  }
  else execvp(cmd -> name, argv);
  return EXIT_FAILURE;
}

static int handle_one_it_tar(command *cmd, char *arg, char *path, char *in_tar,
  char *tar_opt, int not_tar_opt, int nb_valid_arg, int *rest)
{
  if (!not_tar_opt) {
    print_arg_before(cmd, nb_valid_arg, arg);
    int ret = cmd -> in_tar_func(path, in_tar, tar_opt);
    print_arg_after(cmd, rest);
    return ret;
  }
  return EXIT_FAILURE;
}

static void print_arg_after(command *cmd, int *rest)
{
  if (--(*rest) > 0 && cmd -> print_multiple_arg)
    write(STDOUT_FILENO, "\n", 1);
}

static void print_arg_before(command *cmd, int nb_valid_arg, char *arg)
{
  if (cmd -> print_multiple_arg && nb_valid_arg > 1)
  {
    write(STDOUT_FILENO, arg, strlen(arg));
    write(STDOUT_FILENO, ": \n", 3);
  }
}

static int handle_one_it_out(command *cmd, int nb_valid_arg, char **argv, int argc, int *rest)
{
  print_arg_before(cmd, nb_valid_arg, argv[argc]);
  int status, p = fork();
  int ret = EXIT_SUCCESS;
  switch(p)
   {
     case -1:
       error_cmd(cmd -> name, "fork");
       break;
     case 0: // child
       execvp(cmd -> name, argv);
       break;
     default: // parent
       wait(&status);
       if (WEXITSTATUS(status) != EXIT_SUCCESS)
         ret = EXIT_FAILURE;
   }
   print_arg_after(cmd, rest);
   return ret;
}

static int check_options(int argc, char **argv, command *cmd, char *tar_opt, int *tar_opt_c)
{
  int c;
  while ((c = getopt(argc, argv, cmd -> support_opt)) != -1)
  {
    if (c == '?')
    {
      return 1;
    }
    else if (strchr(tar_opt, c) == NULL)
      tar_opt[(*tar_opt_c)++] = c;
  }
  return 0;
}
