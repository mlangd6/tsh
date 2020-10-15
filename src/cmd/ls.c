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
#include <pwd.h>

#define SIZE_OF_LINE 200
#define SIZE_OF_NAME 100

static char *convert_rights_nb_in_ch(char *rights);
static char *is_directory(char c);
static char *give_zero_before(int d);
static char *convert_time(const char *ch);
static char *convert_size(char *size, int t);
static char *concat(char **all, int size);
static char **add_in_line(char **line, struct posix_header ph)

/* Convert a char pointer of rights with cipher format "0000755" in a char pointer format
  "rwxr-xr-x" */
static char *convert_rights_nb_in_ch(char *rights) {
  char *tmp = malloc(10*sizeof(char));
  int c = 1;
  tmp[9] = '\0';
  for(int i = 4; i < 7; i++, c++){
    int aux = c*((i-1)%3)+(c-1)%2;
    switch(rights[i]){
      case '0' : tmp[aux] = '-'; tmp[aux + 1] = '-'; tmp[aux + 2] = '-'; break;
      case '1' : tmp[aux] = '-'; tmp[aux + 1] = '-'; tmp[aux + 2] = 'x'; break;
      case '2' : tmp[aux] = '-'; tmp[aux + 1] = 'w'; tmp[aux + 2] = '-'; break;
      case '3' : tmp[aux] = '-'; tmp[aux + 1] = 'w'; tmp[aux + 2] = 'x'; break;
      case '4' : tmp[aux] = 'r'; tmp[aux + 1] = '-'; tmp[aux + 2] = '-'; break;
      case '5' : tmp[aux] = 'r'; tmp[aux + 1] = '-'; tmp[aux + 2] = 'x'; break;
      case '6' : tmp[aux] = 'r'; tmp[aux + 1] = 'w'; tmp[aux + 2] = '-'; break;
      case '7' : tmp[aux] = 'r'; tmp[aux + 1] = 'w'; tmp[aux + 2] = 'x'; break;
    }
  }
  return tmp;
}

/* Return the letter "d" if the typeflag corresponds with a directory else "-" */
static char *is_directory(char c){
  char *type = malloc(2*sizeof(char));
  type[0] = '-';
  type[1] = '\0';
  if(c == DIRTYPE){
    type[0] = 'd';
  }
  return type;
}

/* Add a '0' before a number if it is between 0 included and 10 excluded and
   return a char pointer representing an integer passed in parameter */
static char *give_zero_before(int d){
  char *res = malloc(3), *buf = malloc(3);
  if(d < 10) {
    strcat(res, "0");
    sprintf(buf, "%d", d);
    strcat(res, buf);
  }
  else {
    sprintf(buf, "%d", d);
    strcat(res, buf);
  }
  return res;
}

/* Convert the time in the format of a number to the format of the human
   time "mmm. dd hh:min" */
static char *convert_time(const char *ch){
  char *c = malloc(12);
  int si;
  strcat(c, ch);
  sscanf(c, "%o", &si);

  int nb_annee_bisextile = (2020 - 1970) /4;
  int min = si/60, minutes = min%60;
  int h = min/60, hours = h%24;
  int d = h/24 - nb_annee_bisextile, day = d%31 ;
  int m = d/31, month = m%12;

  char *res = malloc(20);
  char *buf = malloc(5);

  switch(month){
    case 1 : strcat(res, "jan. "); break;
    case 2 : strcat(res, "feb. "); break;
    case 3 : strcat(res, "mar. "); break;
    case 4 : strcat(res, "apr. "); break;
    case 5 : strcat(res, "may  "); break;
    case 6 : strcat(res, "jun. "); break;
    case 7 : strcat(res, "jul. "); break;
    case 8 : strcat(res, "aug. "); break;
    case 9 : strcat(res, "sep. "); break;
    case 10 : strcat(res, "oct. "); break;
    case 11 : strcat(res, "nov. "); break;
    case 12 : strcat(res, "dec. "); break;
  }
  sprintf(buf, "%d", day);
  strcat(res, buf);
  strcat(res, " ");
  strcat(res, give_zero_before(hours));
  strcat(res, ":");
  strcat(res, give_zero_before(minutes));
  return res;
}

/*
static int color_directory(){
    return -1;
}*/

/* convert the size to octal size */
static char *convert_size(char *size, int t){
  int si = 0, cmp = 0, it = 0;
  char *tmp = malloc(12*sizeof(char));
  sscanf(size, "%011o", &si);
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

/* concatenate the elements of a char pointer pointer between them and add a
   space between each of them */
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

/* Add all the elements for that the command ls -l needs, in a char pointer
   pointer, representing a line of the command */
static char **add_in_line(char **line, struct posix_header ph)
{
  char *unam = malloc(32);
  strcat(unam, ph.uname);
  char *gnam = malloc(32);
  strcat(gnam, ph.gname);
  line[0] = strcat(is_directory(ph.typeflag), convert_rights_nb_in_ch(ph.mode));
  line[1] = unam;
  line[2] = gnam;
  line[3] = convert_size(ph.size, 12);
  line[4] = convert_time(ph.mtime);
  line[5] = ph.name;
  return line;
}

 //TODO : Il manque le nombre de liens pour chaque fichier
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

  for(int i = 0; i < nb_in_tar; i++)
  {
    char *c = NULL;
    if((c = strstr(header[i].name, "/")) == NULL || c[1] == '\0' ){
      char **line = malloc(SIZE_OF_LINE);
      assert(line);
      add_in_line(line, header[i]);
      lines[i] = concat(line, 6);
      strcat(lines[i], "\n");
      if( write(STDOUT_FILENO, lines[i], SIZE_OF_LINE) < 0)
        break;
    }
  }
  close(tar_fd);
  return lines;
}


char **ls(const char *tar_name) {
  struct posix_header *header = tar_ls(tar_name);
  int tar_fd = open(tar_name, O_RDONLY);
  if (tar_fd == -1)
  {
    return error_p(tar_name, &tar_fd, 1);
  }
  int nb_in_tar = nb_files_in_tar(tar_fd);
  char **lines = malloc(nb_in_tar * 100);
  assert(lines);

  for(int i = 0; i < nb_in_tar; i++){
    char *c = NULL;
    if((c = strstr(header[i].name, "/")) == NULL || c[1] == '\0' ){
      lines[i] = strcat(header[i].name, " ");
      write(STDOUT_FILENO, lines[i], 100);
    }
  }
  write(STDOUT_FILENO, "\n", 3);
  close(tar_fd);
  return lines;
}


int main(int argc, char *argv[]){
  if(argc < 3){
    if(is_tar(".") && strcmp(argv[1], "ls") == 0)
      ls(".");
    else exit(EXIT_FAILURE);
  }
  else if(argc == 3){
    if(strcmp(argv[1], "ls") == 0 && strcmp(argv[2], "-l") != 0 && is_tar(argv[2]))
      ls(argv[2]);
    else if(strcmp(argv[1], "ls") == 0 && strcmp(argv[2], "-l") == 0 && is_tar("."))
      ls_l(".");
    else exit(EXIT_FAILURE);
  }
  else if(argc == 4){
    if(strcmp(argv[1], "ls") == 0 && strcmp(argv[2], "-l") == 0 && is_tar(argv[3]))
      ls_l(argv[3]);
    else exit(EXIT_FAILURE);
  }
  return 0;
}
