#include "tar.h"
#include "errors.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#define SIZE_OF_LINE 149

static char *convert_rights_nb_in_ch(char *rights) {
  char *tmp = malloc(10*sizeof(char));
  int c = 1;
  tmp[9] = '\0';
  for(int i = 4; i < 6; i++, c++){
    switch(rights[i]){
      case '0' : tmp[c*(i%3)-1] = '-'; tmp[c*(i%3)] = '-'; tmp[c*(i%3)+1] = '-'; break;
      case '1' : tmp[c*(i%3)-1] = '-'; tmp[c*(i%3)] = '-'; tmp[c*(i%3)+1] = 'x'; break;
      case '2' : tmp[c*(i%3)-1] = '-'; tmp[c*(i%3)] = 'w'; tmp[c*(i%3)+1] = '-'; break;
      case '3' : tmp[c*(i%3)-1] = '-'; tmp[c*(i%3)] = 'w'; tmp[c*(i%3)+1] = 'x'; break;
      case '4' : tmp[c*(i%3)-1] = 'r'; tmp[c*(i%3)] = '-'; tmp[c*(i%3)+1] = '-'; break;
      case '5' : tmp[c*(i%3)-1] = 'r'; tmp[c*(i%3)] = '-'; tmp[c*(i%3)+1] = 'x'; break;
      case '6' : tmp[c*(i%3)-1] = 'r'; tmp[c*(i%3)] = 'w'; tmp[c*(i%3)+1] = '-'; break;
      case '7' : tmp[c*(i%3)-1] = 'r'; tmp[c*(i%3)] = 'w'; tmp[c*(i%3)+1] = 'x'; break;
    }
  }
  return tmp;
}

static char *is_directory(char c){
  char *type = malloc(2*sizeof(char));
  type[0] = '-';
  type[1] = '\0';
  if(c == DIRTYPE){
    type[0] = 'd';
  }
  return type;
}

static char *convert_time(const char *ch){
  struct tm *time = malloc(sizeof(struct tm));
  time = getdate(ch);
  char *tmp = malloc(100*sizeof(char));
  sprintf(tmp, "%d", time->tm_mon);
  return tmp;
}

static int color_directory(){
    return -1;
}

static char *convert_size(char *size, int t){
  int si = 0, cmp = 0, it = 0;
  char *tmp = malloc(12*sizeof(char));
  sscanf(size, "%o", &si);
  for(int i = 0; i < t; i++){
    if(it > 0){
      tmp[cmp++] = size[i];
      it = 1;
    }
    else if(size[i] != '0'){
      tmp[cmp++] = size[i];
      it = 1;
    }
    if(size[i] == '0' && it == 0 && size[i+1] != '\0'){
      tmp[cmp++] = ' ';
      it = 0;
    }
    else if(size[i] == '0' && it == 0 && size[i+1] == '\0'){
      tmp[cmp++] = size[i];
      it = 1;
    }
  }
  tmp[cmp] = tmp[t-1];
  return tmp;
}

static char *concat(char **all, int size)
{
  char *line = malloc(SIZE_OF_LINE);
  for(int i = 0; i < size; i++)
  {
    strcat(line, all[i]);
    strcat(line, " ");
  }
  return line;
}

static char **add_in_line(char **line, struct posix_header ph)
{
  printf("%s\n", convert_time(ph.mtime));
  line[0] = strcat(is_directory(ph.typeflag), convert_rights_nb_in_ch(ph.mode));
  line[1] = ph.uid;
  line[2] = ph.gid;
  line[3] = convert_size(ph.size, 12);
  line[4] = ph.mtime;
  line[5] = ph.name;
  return line;
}

static char *convert_name_user(char *user){
  return NULL;
}

char **ls_l(const char *tar_name) {
  struct posix_header *header = tar_ls(tar_name);
  int tar_fd = open(tar_name, O_RDONLY);
  if (tar_fd == -1)
  {
    return error_p(tar_name, &tar_fd, 1);
  }
  int nb_in_tar = nb_files_in_tar(tar_fd);
  char **lines = malloc(nb_in_tar * SIZE_OF_LINE);
  assert(lines);
  close(tar_fd);

  for(int i = 0; i < nb_in_tar; i++)
  {
    char **line = malloc(6 * 100);
    assert(line);
    add_in_line(line, header[i]);
    lines[i] = concat(line, 6);
    printf("%s\n", lines[i]);
  }
  return lines;
}

char **ls(const char *tar_name) {
  return NULL;
}
