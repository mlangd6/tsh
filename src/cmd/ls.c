#include "tar.h"
#include "errors.h"
#include "path_lib.h"
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
#define CMD_NAME "ls"
#define CMD_NAME_OPT_L "ls -l"

static int convert_rights_nb_in_ch(char *rights);
static int file_type(char c);
static int convert_time(const char *ch);
static char *convert_size(char *size, int t);
static int is_link(char c);
static int nb_link(struct posix_header ph, struct posix_header *header, int n);
static int nb_of_slash(char *name);
static char *tar_name_func(char *name, char *name_two);
static char *cut_name(char *to_cut, char *original, char *new_name);


/* Convert a char pointer of rights with cipher format "0000755" in
   a char pointer format "rwxr-xr-x" */
static int convert_rights_nb_in_ch(char *rights) {
  char tmp[10];
  int c = 1;
  tmp[9] = '\0';
  for(int i = 4; i < 7; i++, c++)
  {
    int aux = c*((i-1)%3)+(c-1)%2;
    switch(rights[i])
    {
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
  tmp[9] = '\0';
  write(STDOUT_FILENO, strcat(tmp, " "), strlen(tmp)+2);
  return 1;
}

/* Return the letter "d" if the typeflag corresponds with a directory,
   "l" if the typeflag corresponds with a link else "-" */
static int file_type(char c) {
  char type[2];
  type[0] = '-';
  type[1] = '\0';
  if(c == DIRTYPE)
    type[0] = 'd';
  if(c == LNKTYPE || c == SYMTYPE)
    type[0] = 'l';
  write(STDOUT_FILENO, type, 2);
  return 1;
}

/* Return 1 if the typeflag representing by char c represent a link type,
   else 0 */
static int is_link(char c) {
  if(c == LNKTYPE || c == SYMTYPE)
    return 1;
  return 0;
}

/* Return the number of occurences of '/' in a name */
static int nb_of_slash(char *name) {
  int cmp = 0;
  for(int i = 0; i < strlen(name)-1; i++)
    if(name[i] == '/') cmp++;
  return cmp;
}

/* Return the number of link for one file of the tar*/
static int nb_link(struct posix_header ph, struct posix_header *header, int n) {
  char nb_ln[10];
  int cmp = 1;
  for(int i = 0; i < n; i++)
    if(strcmp(header[i].linkname, ph.name)==0 ||( header[i].typeflag==DIRTYPE &&
      strstr(header[i].name, ph.name)!=NULL && nb_of_slash(header[i].name)-nb_of_slash(ph.name) == 1) )
      cmp++;
  if(ph.typeflag == DIRTYPE)
    cmp+=1;
  sprintf(nb_ln, "%i", cmp);
  nb_ln[1] = '\0';
  write(STDOUT_FILENO, strcat(nb_ln, "  "), strlen(nb_ln)+3);
  return 1;
}


/* Convert the time in the format of a number to the format of the human
   time "mmm. dd hh:min" */
static int convert_time(const char *ch) {
  char *c = malloc(12);
  int si;
  strcat(c, ch);
  sscanf(c, "%o", &si);
  time_t timestamp =  si;
  struct tm *realtime = gmtime(&timestamp);
  char month[6];
  char day[4];
  char hour[3];
  char min[4];

  switch(realtime->tm_mon)
  {
    case 0 : month[0] = 'j'; month[1] = 'a'; month[2] = 'n'; month[3] = '.'; month[4] = ' '; month[5] = '\0'; break;
    case 1 : month[0] = 'f'; month[1] = 'e'; month[2] = 'b'; month[3] = '.'; month[4] = ' '; month[5] = '\0'; break;
    case 2 : month[0] = 'm'; month[1] = 'a'; month[2] = 'r'; month[3] = '.'; month[4] = ' '; month[5] = '\0'; break;
    case 3 : month[0] = 'a'; month[1] = 'p'; month[2] = 'r'; month[3] = '.'; month[4] = ' '; month[5] = '\0'; break;
    case 4 : month[0] = 'm'; month[1] = 'a'; month[2] = 'y'; month[3] = ' '; month[4] = ' '; month[5] = '\0'; break;
    case 5 : month[0] = 'j'; month[1] = 'u'; month[2] = 'n'; month[3] = '.'; month[4] = ' '; month[5] = '\0'; break;
    case 6 : month[0] = 'j'; month[1] = 'u'; month[2] = 'l'; month[3] = '.'; month[4] = ' '; month[5] = '\0'; break;
    case 7 : month[0] = 'a'; month[1] = 'u'; month[2] = 'g'; month[3] = '.'; month[4] = ' '; month[5] = '\0'; break;
    case 8 : month[0] = 's'; month[1] = 'e'; month[2] = 'p'; month[3] = '.'; month[4] = ' '; month[5] = '\0'; break;
    case 9 : month[0] = 'o'; month[1] = 'c'; month[2] = 't'; month[3] = '.'; month[4] = ' '; month[5] = '\0'; break;
    case 10 : month[0] = 'n'; month[1] = 'o'; month[2] = 'v'; month[3] = '.'; month[4] = ' '; month[5] = '\0'; break;
    case 11 : month[0] = 'd'; month[1] = 'e'; month[2] = 'c'; month[3] = '.'; month[4] = ' '; month[5] = '\0'; break;
    default : exit(EXIT_FAILURE);
  }

  sprintf(day, "%i", (*realtime).tm_mday);
  sprintf(hour, "%i", (*realtime).tm_hour);
  sprintf(min, "%i", (*realtime).tm_min);

  if(strlen(day) == 2) { day[2] = ' '; day[3] ='\0'; }
  else { day[1] = ' '; day[2] = ' '; day[3] = '\0'; }
  if(strlen(min) == 1) { min[1] = min[0]; min[0] = '0'; min[2] = ' '; min[3] = '\0'; }
  else { min[2] = ' '; min[3] = '\0'; }
  if(strlen(hour) == 1) { hour[1] = hour[0]; hour[0] = '0'; }


  write(STDOUT_FILENO, month, strlen(month));
  write(STDOUT_FILENO, day , strlen(day));
  write(STDOUT_FILENO, hour, strlen(hour));
  write(STDOUT_FILENO, ":", 2);
  write(STDOUT_FILENO, min, strlen(min));
  free(c);
  return 1;
}

/*
static int color_directory(){
    return -1;
}*/

/* convert the size to octal size */
static char *convert_size(char *size, int t) {
  int si = 0, cmp = 0, it = 0;
  char *tmp = malloc(12*sizeof(char));
  sscanf(size, "%011o", &si);
  for(int i = 0; i < t; i++) {
    if(it > 0)
    {
      tmp[cmp++] = size[i];
      it = 1;
    }
    else if(size[i] != '0')
    {
      tmp[cmp++] = size[i];
      it = 1;
    }
    if(size[i] == '0' && it == 0 && size[i+1] != '\0')
    {
      tmp[cmp++] = ' ';
      it = 0;
    }
    else if(size[i] == '0' && it == 0 && size[i+1] == '\0')
    {
      tmp[cmp++] = size[i];
      it = 1;
    }
  }
  tmp[cmp] = tmp[t-1];
  write(STDOUT_FILENO, strcat(tmp, "   "), strlen(tmp)+4);
  free(tmp);
  return tmp;
}

/* Return for "/tmp/tsh_test/test.tar/dir1/" the char * "/tmp/tsh_test/test.tar" */
static char *tar_name_func(char *name, char *n){
  int j = 0;
  if( strstr(name, ".tar") == NULL){
    return NULL;
  }
  for(int i = 0; i < strlen(name); i++){
    if(name[i] == '.' && name[i+1] == 't' && name[i+2] == 'a' && name[i+3] == 'r'){
      n[j] = name[i];
      n[j+1] = name[i+1];
      n[j+2] = name[i+2];
      n[i+3] = name[i+3];
      break;
    }else
      n[j++] = name[i];
  }
  return n;
}

/* remove original in to_cut and stock this result in new_name which is return */
static char *cut_name(char *to_cut, char *original, char *new_name){
  int i = 0, j = 0;
  int size = strlen(to_cut) - strlen(original) + 1;
  while(to_cut[i] == original[i]){
    i++;
  }
  for(j = 0; j < size; j++){
    new_name[j] = to_cut[i++];
  }
  new_name[j] = '\0';
  return new_name;
}


int ls_l(char *tar_name, char *name_in_tar) {
  char *n = malloc(100);
  char *effective_tar_name = tar_name_func(tar_name, n);
  struct posix_header *header = tar_ls(effective_tar_name);
  int tar_fd = open(effective_tar_name, O_RDONLY);
  if (tar_fd == -1)
    return error_pt(tar_name, &tar_fd, 1);
  int nb_in_tar = nb_files_in_tar(tar_fd);


  if(strcmp(name_in_tar, "\0") != 0){
    for(int i = 0; i < nb_in_tar; i++)
    {
      char *c = NULL;
      char *d = NULL;
      char *new_name = malloc(strlen(name_in_tar));
      cut_name(header[i].name, name_in_tar, new_name);
      if(strcmp(name_in_tar, header[i].name)!=0
      && ((d = strstr(header[i].name, name_in_tar)) != NULL)
      && name_in_tar[strlen(name_in_tar)-1] == '/'
      && name_in_tar[strlen(name_in_tar)] == '\0'
      && ((c = strstr(new_name, "/")) == NULL || c[1] == '\0' ))
      {
        file_type(header[i].typeflag);
        convert_rights_nb_in_ch(header[i].mode);
        write(STDOUT_FILENO, " ", 2);
        nb_link(header[i], header, nb_in_tar);
        write(STDOUT_FILENO, strcat(header[i].uname, " "), strlen(header[i].uname)+2);
        write(STDOUT_FILENO, strcat(header[i].gname, " "), strlen(header[i].gname)+2);
        convert_size(header[i].size, 12);
        convert_time(header[i].mtime);
        if(is_link(header[i].typeflag))
          write(STDOUT_FILENO, strcat(strcat(header[i].name, " -> "), header[i].linkname), strlen(header[i].name)+5+strlen(header[i].linkname));
        else
          write(STDOUT_FILENO, strcat(new_name, " "), strlen(new_name)+2);
        write(STDOUT_FILENO, "\n", 2);
      }
      free(new_name);
    }
  }else{
    for(int i = 0; i < nb_in_tar; i++)
    {
      char *c = NULL;
      if((c = strstr(header[i].name, "/")) == NULL || c[1] == '\0' )
      {
        file_type(header[i].typeflag);
        convert_rights_nb_in_ch(header[i].mode);
        write(STDOUT_FILENO, " ", 2);
        nb_link(header[i], header, nb_in_tar);
        write(STDOUT_FILENO, strcat(header[i].uname, " "), strlen(header[i].uname)+2);
        write(STDOUT_FILENO, strcat(header[i].gname, " "), strlen(header[i].gname)+2);
        convert_size(header[i].size, 12);
        convert_time(header[i].mtime);
        if(is_link(header[i].typeflag))
          write(STDOUT_FILENO, strcat(strcat(header[i].name, " -> "), header[i].linkname), strlen(header[i].name)+5+strlen(header[i].linkname));
        else
          write(STDOUT_FILENO, strcat(header[i].name, " "), strlen(header[i].name));
        write(STDOUT_FILENO, "\n", 2);
      }
    }
  }
  free(n);
  close(tar_fd);
  return 1;
}

int ls(char *tar_name, char *name_in_tar) {
  char *effective_tar_name = tar_name_func(tar_name, name_in_tar);
  name_in_tar = split_tar_abs_path(tar_name);
  struct posix_header *header = tar_ls(effective_tar_name);
  int tar_fd = open(effective_tar_name, O_RDONLY);
  if (tar_fd == -1)
    return error_pt(tar_name, &tar_fd, 1);
  int nb_in_tar = nb_files_in_tar(tar_fd);
  int empty = 1;

  if(strcmp(name_in_tar, "\0") != 0){
    for(int i = 0; i < nb_in_tar; i++)
    {
      char *c = NULL;
      char *d = NULL;
      char *new_name = malloc(strlen(name_in_tar));
      cut_name(header[i].name, name_in_tar, new_name);
      if(strcmp(name_in_tar, header[i].name)!=0
      && ((d = strstr(header[i].name, name_in_tar)) != NULL && name_in_tar[strlen(name_in_tar)-1] == '/' && name_in_tar[strlen(name_in_tar)] == '\0')
      && ((c = strstr(new_name, "/")) == NULL || c[1] == '\0' )) {
        write(STDOUT_FILENO, strcat(new_name, "   "), strlen(new_name)+4);
        empty = 0;
      }
      if(i == nb_in_tar-1 && !empty)
        write(STDOUT_FILENO, "\n", 2);
      free(new_name);
    }
  }
  else {
    for(int i = 0; i < nb_in_tar; i++)
    {
      char *c = NULL;
      if((c = strstr(header[i].name, "/")) == NULL || c[1] == '\0' ){
        write(STDOUT_FILENO, strcat(header[i].name, "   "), strlen(header[i].name)+4);
        empty = 0;
      }
      if(i == nb_in_tar-1 && !empty)
        write(STDOUT_FILENO, "\n", 2);
    }
  }
  close(tar_fd);
  return 0;
}

//GERER LE CAS OU LE NOMBRE D'ARGUMENTS EST SUPERIEUR A 3
int main(int argc, char *argv[]) {
  char *name = malloc(100);
  char *tar = malloc(100);
  if(argc == 1 ) {
    execlp(CMD_NAME, CMD_NAME, NULL);
  }
  else if(argc == 2)
  {
    if(strcmp(argv[1], "-l")==0)
      execvp(CMD_NAME, argv);
    else if(argv[1][0] != '-' && is_tar(tar_name_func(argv[1], tar)) != 1)
      execvp(CMD_NAME, argv);
    else if(strcmp(argv[1], "-l")!=0 && is_tar(tar_name_func(argv[1], tar)) == 1)
      ls(argv[1], name);
    else
      write(STDOUT_FILENO, "ls : error\n" , 12);
  }
  else if(argc == 3)
  {
    name = split_tar_abs_path(argv[2]);
    if(strcmp(argv[1], "-l") == 0 && is_tar(tar_name_func(argv[2], tar)) != 1)
      execvp(CMD_NAME, argv);
    else if(strcmp(argv[1], "-l") == 0 && is_tar(tar_name_func(argv[2], tar)) == 1 && strcmp(name, "\0") != 0)
      ls_l(argv[2], name);
    else if(strcmp(argv[1], "-l") == 0 && is_tar(tar_name_func(argv[2], tar)) == 1)
      ls_l(argv[2], "");
    else
      write(STDOUT_FILENO, "ls : error\n" , 12);
  }
  /*else{
    if(strcmp(argv[1], "-l") == 0){
      printf("A\n");
      for(int i = 2; i < argc; i++){
        printf("%d %s\n", i, argv[i]);
        name = split_tar_abs_path(argv[i]);
        if(is_tar(tar_name_func(argv[i], tar)) == 1){
          printf("%d - 1\n", i);
          write(STDOUT_FILENO, name, strlen(name));
          write(STDOUT_FILENO, " :\n", 4);
          ls_l(argv[i], name);
        }
        else{
          printf("%s %d - 2\n",argv[i], i);
          write(STDOUT_FILENO, argv[i], strlen(argv[i]));
          write(STDOUT_FILENO, " :\n", 4);
          execlp(CMD_NAME, CMD_NAME, argv[1], argv[i], NULL);
        }
      }
    }else{
      printf("B\n");
      for(int i = 1; i < argc; i++){
        printf("%d\n", i);
        name = split_tar_abs_path(argv[i]);
        if(is_tar(tar_name_func(argv[i], tar)) == 1){
          printf("%d - 1\n", i);
          write(STDOUT_FILENO, name, strlen(name));
          write(STDOUT_FILENO, " :\n", 4);
          ls(argv[i], name);
        }
        else{
          printf("%d - 2\n", i);
          write(STDOUT_FILENO, argv[i], strlen(argv[i]));
          write(STDOUT_FILENO, " :\n", 4);
          execlp(CMD_NAME, CMD_NAME, argv[i], NULL);
        }
      }
    }
  }*/
  free(name);
  free(tar);
  return 0;
}
