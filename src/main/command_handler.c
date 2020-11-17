#include <linux/limits.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

#include "errors.h"
#include "path_lib.h"
#include "tar.h"
#include "command_handler.h"

static void invalid_options(char *cmd_name);
static void parse_args(int argc, char **argv, arg_info *info);
static int handle_with_pwd(command *cmd, char **argv, int not_tar_opt, char *tar_opt);
static int handle_one_it_tar(command *cmd, char *arg, char *cpy, char *in_tar,
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
    calloc(argc, sizeof(char *))
  };
  info.options[info.opt_c++] = cmd.name;
  parse_args(argc, argv, &info);
  if (info.nb_in_tar == 0 && (info.nb_out > 0 || !cmd.twd_arg))
    execvp(cmd.name, argv);
  opterr = 0;
  int ret = EXIT_SUCCESS;
  int sup_opt_len = strlen(cmd.support_opt);
  char *tar_opt = malloc(sup_opt_len  + 1);
  memset(tar_opt, '\0', sup_opt_len + 1);
  int tar_opt_c = 0;
  int not_tar_opt = check_options(argc, argv, &cmd, tar_opt, &tar_opt_c);
  if (info.nb_in_tar == 0 && info.nb_out == 0) // No arguments (cmd.twd_arg == 1)
    return handle_with_pwd(&cmd, argv, not_tar_opt, tar_opt);

  int rest = info.nb_out + ((not_tar_opt) ? 0: info.nb_in_tar);
  int nb_valid_arg = rest;
  char *in_tar;
  for (int i = 1; i < argc; i++)
  {
    if (*argv[i] == '-')
      continue;
    char cpy[PATH_MAX];
    memcpy(cpy, argv[i], strlen(argv[i]) + 1);
    in_tar = split_tar_abs_path(argv[i]);
    if (*in_tar != '\0' || is_tar(argv[i]) == 1) // Tar involved
    {
      if (handle_one_it_tar(&cmd, argv[i], cpy, in_tar, tar_opt, not_tar_opt, nb_valid_arg, &rest) == EXIT_FAILURE)
        ret = EXIT_FAILURE;
    }
    else
    {
      info.options[info.opt_c] = argv[i];
      if (handle_one_it_out(&cmd, nb_valid_arg, info.options, info.opt_c, &rest) == EXIT_FAILURE)
        ret = EXIT_FAILURE;
      info.options[info.opt_c] = NULL;
    }

  }
  free(info.options);
  return ret;
}

static void parse_args(int argc, char **argv, arg_info *info)
{
  char cpy[PATH_MAX];
  for (size_t len, i = 1; i < argc; i++)
  {
    len = strlen(argv[i]) + 1;
    if (*argv[i] == '-' && ++(info -> opt_c))
      info -> options[info -> opt_c-1] = argv[i];
    else
    {
      memmove(cpy, argv[i], len);
      char *in_tar = split_tar_abs_path(cpy);
      if ( is_tar(cpy) == 1 || *in_tar != '\0')
        info -> nb_in_tar++;
      else info -> nb_out++;
    }
  }
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
  if (*in_tar != '\0' || is_tar(twd) == 1)
    return (not_tar_opt) ? EXIT_FAILURE : cmd -> in_tar_func(twd, in_tar, tar_opt);
  else execvp(cmd -> name, argv);
  return EXIT_FAILURE;
}

static int handle_one_it_tar(command *cmd, char *arg, char *cpy, char *in_tar,
  char *tar_opt, int not_tar_opt, int nb_valid_arg, int *rest)
{
  if (!not_tar_opt) {
    print_arg_before(cmd, nb_valid_arg, cpy);
    int ret = cmd -> in_tar_func(arg, in_tar, tar_opt);
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
      invalid_options(cmd -> name);
      return 1;
    }
    else if (strchr(tar_opt, c) == NULL)
      tar_opt[(*tar_opt_c)++] = c;
  }
  return 0;
}
