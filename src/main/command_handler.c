#include <linux/limits.h>
#include <string.h>


#include "path_lib.h"
#include "tar.h"
#include "command_handler.h"

int handle(command c) {
  return 0;
}

int has_tar_arg(int argc, char **argv)
{
  int ret = 2;
  char cpy[PATH_MAX];
  for (size_t len, i = 1; i < argc; i++)
  {
    len = strlen(argv[i]) + 1;
    if (*argv[i] == '-')
      continue;
    memmove(cpy, argv[i], len);
    char *in_tar = split_tar_abs_path(cpy);
    if ( is_tar(cpy) == 1 || *in_tar != '\0') {
      return 1;
    }
    ret = 0;
  }
  return ret;
}
