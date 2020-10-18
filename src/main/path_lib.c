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
      return chr+1;
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


char *reduce_abs_path(char *path) {
  char **prev_chr = malloc(strlen(path) * sizeof(char *)); // Maximum number of '/'
  char *chr = path;
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
  free(prev_chr);
  return path;
}
