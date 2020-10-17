#include <string.h>
#include <stdlib.h>
#include "tar.h"

char *split_tar_abs_path(char *path) {
  if (path[0] != '/') {
    return NULL;
  }
  char *chr = path+1;

  while ( (chr = strchr(chr, '/')) != NULL) {
    *chr = '\0';
    if (is_tar(path) == 1) {
      return path + (chr + 1 - path);
    }
    *chr = '/';
    chr++;
  }
  return strchr(path, '\0');
}

/* Returns 0 if the first two char represents a "." dir,
    1 if the first three char represents the parent ".." dir,
    -1 else. */
static int is_special_dir(char const *s) {
  if (s[0] == '.') {
    if (s[1] == '\0' || s[1] == '/') {
      return 0;
    }
    if (s[1] == '.' && (s[2] == '/' || s[2] == '\0')) {
      return 1;
    }
  }
  return -1;
}


char *reduce_abs_path(char const *path) {
  int len = strlen(path);
  char *res = malloc(len + 1); // Max size possible
  char **prev_chr = malloc(len * sizeof(char *)); // Maximum number of '/'
  strcpy(res, path);
  char *chr = res;
  int i = 0;
  while ( (chr = strchr(chr, '/')) != NULL) {
    switch (is_special_dir(++chr)) {
      case 0:
        strcpy(chr, chr + 2);
        chr--;
        break;
      case 1:
        if (prev_chr[0] == NULL) { // path starts by /.. ( == /)
          strcpy(chr, chr + 3);
        }
        else {
          if (chr[2] == '\0'){
            strcpy(prev_chr[--i], chr + 2);
          }
          else {
            strcpy(prev_chr[--i], chr + 3);
          }
          chr = prev_chr[i] - 1;
          prev_chr[i] = NULL;
        }
        break;
      default:
        prev_chr[i++] = chr;
    }
  }
  return res;
}

char *reduce_path_and_split(char const *path) {
//   if (path == NULL) {
//     return NULL;
//   }
//   if (path[0] == '/') {
//     return split_tar_abs_path(reduce_abs_path(*path));
//   }
  return NULL;
}
