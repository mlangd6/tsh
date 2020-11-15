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
  int c;
  opterr = 0;
  int ret = EXIT_SUCCESS;
  int not_tar_opt = 0;
  int sup_opt_len = strlen(cmd.support_opt);
  char *tar_opt = malloc(sup_opt_len  + 1);
  memset(tar_opt, '\0', sup_opt_len + 1);
  int tar_opt_c = 0;
  while ((c = getopt(argc, argv, cmd.support_opt)) != -1)
  {

    if (c == '?')
    {
      write(STDERR_FILENO, cmd.name, strlen(cmd.name));
      write(STDERR_FILENO, ": invalid option -- '", 21);
      write(STDERR_FILENO, &optopt, 1);
      write(STDERR_FILENO, "' with tarball, skipping files inside tarball\n", 46);
      not_tar_opt = 1;
      break;
    }
    else if (strchr(tar_opt, c) == NULL)
      tar_opt[tar_opt_c++] = c;
  }
  if (info.nb_in_tar == 0 && info.nb_out == 0) // No arguments (cmd.twd_arg == 1)
  {
    char *tmp = getenv("PWD");
    char twd[PATH_MAX];
    memcpy(twd, tmp, strlen(tmp) + 1);
    char *in_tar = split_tar_abs_path(twd);
    if (*in_tar != '\0' || is_tar(twd) == 1)
    {
      return (not_tar_opt) ? EXIT_FAILURE : cmd.in_tar_func(twd, in_tar, tar_opt);
    }
    else execvp(cmd.name, argv);
  }
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
    if (*in_tar != '\0' || is_tar(argv[i]) == 1)
    {
      if (!not_tar_opt) {
        if (cmd.print_multiple_arg && nb_valid_arg > 1)
        {
          write(STDOUT_FILENO, cpy, strlen(cpy));
          write(STDOUT_FILENO, ": \n", 3);
        }
        cmd.in_tar_func(argv[i], in_tar, tar_opt);
        if (--rest > 0 && cmd.print_multiple_arg)
          write(STDOUT_FILENO, "\n", 1);
      }
    }
    else
    {
      if (cmd.print_multiple_arg && nb_valid_arg > 1)
      {
        write(STDOUT_FILENO, argv[i], strlen(argv[i]));
        write(STDOUT_FILENO, ":\n", 2);
      }
      info.options[info.opt_c] = argv[i];
      int status, p = fork();
      switch (p)
      {
        case -1:
          error_cmd(cmd.name, "fork");
          break;
        case 0: // child
          execvp(cmd.name, info.options);
          break;
        default: // parent
          wait(&status);
          if (WEXITSTATUS(status) != EXIT_SUCCESS)
            ret = EXIT_FAILURE;
      }
      info.options[info.opt_c] = NULL;
      if (--rest > 0 && cmd.print_multiple_arg)
        write(STDOUT_FILENO, "\n", 1);
    }

  }
  free(info.options);
  return ret;
}

void parse_args(int argc, char **argv, arg_info *info)
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
