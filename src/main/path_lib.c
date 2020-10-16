#include <string.h>
#include <linux/limits.h>
#include <stdlib.h>
#include "tar.h"

char *split_tar_abs_path(char const *path) {
  if (path[0] != '/') {
    return NULL;
  }
  char *cpy = malloc(strlen(path) + 1);
  strcpy(cpy, path);
  char *chr = cpy+1;

  while ( (chr = strchr(chr, '/')) != NULL) {
    *chr = '\0';
    if (is_tar(cpy) == 1) {
      #pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
      char *res = path + (chr + 1 - cpy);
      free(cpy);
      return res;
    }
    *chr = '/';
    chr++;
  }
  free(cpy);
  return strchr(path, '\0');
}
