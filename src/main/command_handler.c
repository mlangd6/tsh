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

static int tar_arg_and_options(int argc, char **argv, int *opt_c, char **options);

int handle(command cmd, int argc, char **argv) {
  int opt_c;
  char **options = calloc(argc, sizeof(char *));
  int tar_arg = tar_arg_and_options(argc, argv, &opt_c, options);
  if (!tar_arg || (tar_arg == 2 && !cmd.twd_arg))
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
  if (tar_arg == 2) // No arguments (cmd.twd_arg == 1)
  {
    char *tmp = getenv("PWD");
    char twd[PATH_MAX];
    memcpy(twd, tmp, strlen(tmp) + 1);
    char *in_tar = split_tar_abs_path(twd);
    if (*in_tar != '\0' || is_tar(twd) == 1)
      return (not_tar_opt) ? EXIT_FAILURE : cmd.in_tar_func(twd, in_tar, tar_opt);

  }
  // tar_arg == 1
  char *in_tar;
  for (int i = 1; i < argc; i++)
  {
    if (*argv[i] == '-')
      continue;
    in_tar = split_tar_abs_path(argv[i]);
    if (*in_tar != '\0' || is_tar(argv[i]) == 1)
    {
      if (!not_tar_opt && cmd.in_tar_func(argv[i], in_tar, tar_opt) == EXIT_FAILURE)
        ret = EXIT_FAILURE;
    }
    else
    {
      options[opt_c] = argv[i];
      int status, p = fork();
      switch (p)
      {
        case -1:
          error_cmd(cmd.name, "fork");
          break;
        case 0: // child
          execvp(cmd.name, options);
          break;
        default: // parent
          wait(&status);
          if (WEXITSTATUS(status) != EXIT_SUCCESS)
            ret = EXIT_FAILURE;
      }
      options[opt_c] = NULL;
    }
  }
  free(options);
  return ret;
}

static int tar_arg_and_options(int argc, char **argv, int *opt_c, char **options)
{
  int ret = 2;
  char cpy[PATH_MAX];
  for (size_t len, i = 1; i < argc; i++)
  {
    len = strlen(argv[i]) + 1;
    if (*argv[i] == '-' && ++(*opt_c))
      options[*(opt_c)-1] = argv[i];
    else if (ret != 1)
    {
      memmove(cpy, argv[i], len);
      char *in_tar = split_tar_abs_path(cpy);
      if ( is_tar(cpy) == 1 || *in_tar != '\0')
        ret = 1;
      else ret = 0;
    }
  }
  return ret;
}
