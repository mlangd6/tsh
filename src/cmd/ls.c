#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "command_handler.h"
#include "errors.h"
#include "path_lib.h"
#include "tar.h"
#include "utils.h"

/** Supported options by ls */
#define SUPPORT_OPT ""

/** Command name */
#define CMD_NAME "ls"

/* static int max_nlink_width; */
/* static int max_owner_width; */
/* static int max_group_width; */
/* static int max_size_width; */

/* static int total_block; */

static char *get_corrected_name (const char *tar_name, const char *filename); 

static int print_string(const char *string);

static int print_files (array *files, bool long_format);
static int print_header (struct posix_header *header, bool long_format, bool newline);

/* static int convert_rights_nb_in_ch(char *rights); */
/* static int file_type(char c); */
/* static int convert_time(const char *ch); */
/* static int convert_size(char *size, int t); */
/* static int is_link(char c); */
/* static int nb_link(struct posix_header ph, struct posix_header *header, int n); */
/* static int nb_of_slash(char *name); */
/* static char *cut_name(char *to_cut, char *original, char *the_file_to_list); */
/* static int is_file(struct posix_header *header, char *name_in_tar, int nb_of_files_in_tar, int opt_or_not); */
/* static int write_ls(struct posix_header *header, struct posix_header header_to_write, char *name, int nb_of_files_in_tar); */
/* static int ls_(char *tar_name, char *in_tar_name); */
/* static int ls_l(char *tar_name, char*filename); */



static char *get_corrected_name (const char *tar_name, const char *filename)
{  
  char *corrected_name = malloc(strlen(filename)+2); // 2 = 1 (\0 à la fin) + 1 (ajout possible d'un /)
  if (!corrected_name)
    return NULL;

  strcpy(corrected_name, filename);

  // filename la racine ou un nom de dossier
  if (*filename == '\0' || is_dir_name(filename))
    return corrected_name;
  
  // filename existe dans le tar
  if (tar_access(tar_name, corrected_name, F_OK) > 0)
    return corrected_name;

  // filename/ existe dans le tar
  strcat(corrected_name, "/");
  if (tar_access(tar_name, corrected_name, F_OK) > 0)
    return corrected_name;

  // filename n'existe sous aucune forme dans le tar
  free(corrected_name);
  return NULL;
}


static int print_string(const char *string)
{
  return write(STDOUT_FILENO, string, strlen(string)+1);
}


static int print_files (array *files, bool long_format)
{
  if (array_size(files) == 0)
    return 0;
  
  tar_file *tf;
  int i;
  
  for (i=0; i < array_size(files)-1; i++)
    {
      tf = array_get(files, i);
      
      if (long_format)
	{
	  print_header (&tf->header, long_format, true);
	}
      else
	{
	  print_header (&tf->header, long_format, false);
	  print_string("  ");
	}
      
      free(tf);
    }

  tf = array_get(files, i);
  print_header (&tf->header, long_format, true);
  free(tf);
  
  return 0;
}

static int print_header (struct posix_header *header, bool long_format, bool newline)
{
  if (long_format)
    {

    }
  else
    {
      print_string(header->name);      
    }
  
  if (newline)
    print_string("\n");

  return 0;
}


/* /\* Convert a char pointer of rights with cipher format "0000755" in */
/*    a char pointer format "rwxr-xr-x" *\/ */
/* static int convert_rights_nb_in_ch(char *rights) { */
/*   char *tmp = calloc(11, 1); */
/*   int c = 1; */
/*   for(int i = 4; i < 7; i++, c++) */
/*     { */
/*       int aux = c*((i-1)%3)+(c-1)%2; */
/*       switch(rights[i]) */
/* 	{ */
/* 	case '0' : tmp[aux] = '-'; tmp[aux + 1] = '-'; tmp[aux + 2] = '-'; break; */
/* 	case '1' : tmp[aux] = '-'; tmp[aux + 1] = '-'; tmp[aux + 2] = 'x'; break; */
/* 	case '2' : tmp[aux] = '-'; tmp[aux + 1] = 'w'; tmp[aux + 2] = '-'; break; */
/* 	case '3' : tmp[aux] = '-'; tmp[aux + 1] = 'w'; tmp[aux + 2] = 'x'; break; */
/* 	case '4' : tmp[aux] = 'r'; tmp[aux + 1] = '-'; tmp[aux + 2] = '-'; break; */
/* 	case '5' : tmp[aux] = 'r'; tmp[aux + 1] = '-'; tmp[aux + 2] = 'x'; break; */
/* 	case '6' : tmp[aux] = 'r'; tmp[aux + 1] = 'w'; tmp[aux + 2] = '-'; break; */
/* 	case '7' : tmp[aux] = 'r'; tmp[aux + 1] = 'w'; tmp[aux + 2] = 'x'; break; */
/* 	} */
/*     } */
/*   tmp[9] = ' ';tmp[10] = '\0'; */
/*   write(STDOUT_FILENO, tmp, strlen(tmp)); */
/*   free(tmp); */
/*   return 0; */
/* } */

/* /\* Return the letter "d" if the typeflag corresponds with a directory, */
/*    "l" if the typeflag corresponds with a link else "-" *\/ */
/* static int file_type(char c) { */
/*   if(c == DIRTYPE) */
/*     write(STDOUT_FILENO, "d", 1); */
/*   else if(c == LNKTYPE || c == SYMTYPE) */
/*     write(STDOUT_FILENO, "l", 1); */
/*   else */
/*     write(STDOUT_FILENO, "-", 1); */
/*   return 0; */
/* } */

/* /\* Return 1 if the typeflag representing by char c represent a link type, */
/*    else 0 *\/ */
/* static int is_link(char c) { */
/*   if(c == LNKTYPE || c == SYMTYPE) */
/*     return 1; */
/*   return 0; */
/* } */

/* /\* Return the number of occurences of '/' in a name *\/ */
/* static int nb_of_slash(char *name) { */
/*   int cmp = 0; */
/*   for(int i = 0; i < strlen(name)-1; i++) */
/*     if(name[i] == '/') */
/*       cmp++; */
/*   return cmp; */
/* } */

/* /\* Return the number of link for one file of the tar*\/ */
/* static int nb_link(struct posix_header ph, struct posix_header *header, int n) { */
/*   char *nb_ln = malloc(10); */
/*   int cmp = 1; */
/*   for(int i = 0; i < n; i++) */
/*     if( strcmp(header[i].linkname, ph.name)==0 ||( header[i].typeflag==DIRTYPE */
/* 						   && strstr(header[i].name, ph.name)!=NULL */
/* 						   && nb_of_slash(header[i].name)-nb_of_slash(ph.name) == 1) ) */
/*       cmp++; */
/*   if(ph.typeflag == DIRTYPE) */
/*     cmp++; */
/*   sprintf(nb_ln, "%i", cmp); */
/*   strcat(nb_ln, "  "); */
/*   write(STDOUT_FILENO, nb_ln, strlen(nb_ln)); */
/*   free(nb_ln); */
/*   return 0; */
/* } */


/* /\* Convert the time in the format of a number to the format of the human */
/*    time "mmm. dd hh:min" *\/ */
/* static int convert_time(const char *ch) */
/* { */
/*   int si; */
/*   sscanf(ch, "%o", &si); */
/*   time_t timestamp =  si; */
/*   struct tm *realtime = gmtime(&timestamp); */

/*   char *month = calloc(6, 1); */
/*   char *day   = calloc(4, 1); */
/*   char *hour  = calloc(4,1); */
/*   char *min   = calloc(4,1); */

/*   switch(realtime->tm_mon) */
/*     { */
/*     case 0 : strcat(month, "jan. "); break; */
/*     case 1 : strcat(month, "feb. "); break; */
/*     case 2 : strcat(month, "mar. "); break; */
/*     case 3 : strcat(month, "apr. "); break; */
/*     case 4 : strcat(month, "may  "); break; */
/*     case 5 : strcat(month, "jun. "); break; */
/*     case 6 : strcat(month, "jul. "); break; */
/*     case 7 : strcat(month, "aug. "); break; */
/*     case 8 : strcat(month, "sep. "); break; */
/*     case 9 : strcat(month, "oct. "); break; */
/*     case 10 : strcat(month, "nov. "); break; */
/*     case 11 : strcat(month, "dec. "); break; */
/*     default : exit(EXIT_FAILURE); */
/*     } */

/*   sprintf(day, "%i", (*realtime).tm_mday); */
/*   sprintf(hour, "%i", (*realtime).tm_hour); */
/*   sprintf(min, "%i", (*realtime).tm_min); */

/*   if(strlen(day) == 2) */
/*     { day[2] = ' '; */
/*       day[3] ='\0'; */
/*     } */
/*   else */
/*     { day[1] = ' '; */
/*       day[2] = ' '; */
/*       day[3] = '\0'; */
/*     } */
  
/*   if(strlen(hour) == 1) */
/*     { */
/*       hour[1] = hour[0]; */
/*       hour[0] = '0'; */
/*       hour[2] = ':'; */
/*       hour[3] = '\0'; */
/*     } */
/*   else */
/*     { */
/*       hour[2] = ':'; */
/*       hour[3] = '\0'; */
/*     } */
  
/*   if(strlen(min) == 1) */
/*     { */
/*       min[1] = min[0]; */
/*       min[0] = '0'; */
/*       min[2] = ' '; */
/*       min[3] = '\0'; */
/*     } */
/*   else */
/*     { */
/*       min[2] = ' '; */
/*       min[3] = '\0'; */
/*     } */

/*   strcat(month, strcat(day, strcat(hour, min))); */
/*   //month is equivalent to the entire date now */
/*   write(STDOUT_FILENO, month, strlen(month)); */

/*   free(month); free(day); free(hour); free(min); */
/*   return 0; */
/* } */

/* /\* convert the size to octal size *\/ */
/* static int convert_size(char *size, int t) { */
/*   int si = 0, cmp = 0, more_than_zero_before = 0; */
/*   char *tmp = malloc(12); */
/*   sscanf(size, "%o", &si); */
/*   for(int i = 0; i < t; i++) { */
/*     if(more_than_zero_before > 0 || size[i] != '0' || (size[i] == '0' && more_than_zero_before == 0 && size[i+1] == '\0')) */
/*       { */
/* 	tmp[cmp++] = size[i]; */
/* 	more_than_zero_before = 1; */
/*       } */
/*     if(size[i] == '0' && more_than_zero_before == 0 && size[i+1] != '\0') */
/*       tmp[cmp++] = ' '; */
/*   } */
/*   tmp[cmp] = tmp[t-1]; */
/*   strcat(tmp, "   "); */
/*   write(STDOUT_FILENO, tmp, strlen(tmp)); */
/*   free(tmp); */
/*   return 0; */
/* } */

/* /\* remove the name of origin "original" in to_cut and stock this result in the_file_to_list which is return */
/*    example : cut_name("tttt.tar/toto/tata/titi", "tttt.tar", "") == "titi"   *\/ */
/* static char *cut_name(char *to_cut, char *original, char *the_file_to_list){ */
/*   int i = 0, j = 0; */
/*   int size = strlen(to_cut) - strlen(original) + 1; */
/*   while(to_cut[i] == original[i]){ */
/*     i++; */
/*   } */
/*   for(j = 0; j < size; j++){ */
/*     the_file_to_list[j] = to_cut[i++]; */
/*   } */
/*   the_file_to_list[j] = '\0'; */
/*   return the_file_to_list; */
/* } */

/* /\* list the name if the name input is a file. Return boolean. *\/ */
/* static int is_file(struct posix_header *header, char *name_in_tar, int nb_of_files_in_tar, int opt_or_not){ */
/*   for(int i = 0; i < nb_of_files_in_tar; i++){ */
/*     if(strcmp(name_in_tar, header[i].name) == 0 && header[i].typeflag != '5'){ */
/*       if(opt_or_not == 1) write_ls(header, header[i], name_in_tar, nb_of_files_in_tar); */
/*       if(opt_or_not == 0) { */
/*         write(STDOUT_FILENO, name_in_tar, strlen(name_in_tar)); */
/*         write(STDOUT_FILENO, "\n", 1); */
/*       } */
/*       return 1; */
/*     } */
/*   } */
/*   return 0; */
/* } */

/* /\* Write all the informations for "ls -l" *\/ */
/* static int write_ls(struct posix_header *header, struct posix_header header_to_write, char *name, int nb_of_files_in_tar){ */
/*   file_type(header_to_write.typeflag); */
/*   convert_rights_nb_in_ch(header_to_write.mode); */
/*   write(STDOUT_FILENO, " ", 1); */
/*   nb_link(header_to_write, header, nb_of_files_in_tar); */
/*   write(STDOUT_FILENO, header_to_write.uname, strlen(header_to_write.uname)); */
/*   write(STDOUT_FILENO, " ", 1); */
/*   write(STDOUT_FILENO, header_to_write.gname, strlen(header_to_write.gname)); */
/*   write(STDOUT_FILENO, " ", 1); */
/*   convert_size(header_to_write.size, 12); */
/*   convert_time(header_to_write.mtime); */
/*   if(is_link(header_to_write.typeflag)) */
/*     { */
/*       write(STDOUT_FILENO, header_to_write.name, strlen(header_to_write.name)); */
/*       write(STDOUT_FILENO, " -> ", 4); */
/*       write(STDOUT_FILENO, header_to_write.linkname, strlen(header_to_write.linkname)); */
/*     } */
/*   else */
/*     write(STDOUT_FILENO, name, strlen(name)); */
/*   write(STDOUT_FILENO, "\n", 1); */
/*   return 0; */
/* } */

/* Representing the command ls -l which shows the type of the files, rights,
   the number of links the user name, group name, size, last modification
   date, and the file name. Take a char * in parameter representing the
   file .tar that we want to list */
/* int ls_l(char *tar_name, char *name_in_tar) */
/* {   */
/*   int nb_of_files_in_tar = 0; */
/*   struct posix_header *header = tar_ls(tar_name, &nb_of_files_in_tar); */
/*   int tar_fd = open(tar_name, O_RDONLY); */
/*   if (tar_fd == -1) */
/*     { */
/*     error_cmd(CMD_NAME, tar_name); */
/*     return error_pt(&tar_fd, 1, errno); */
/*   } */

/*   if(is_file(header, name_in_tar, nb_of_files_in_tar, 1)) */
/*     { */
/*       close(tar_fd); */
/*       return 0; */
/*     } */
/*   if(*name_in_tar != '\0') { */
/*     if(name_in_tar[strlen(name_in_tar)-1] != '/'){ strcat(name_in_tar, "/");} */
/*     if (tar_access(tar_name, name_in_tar, R_OK) == -1) */
/*       { */
/* 	name_in_tar[-1] = '/'; */
/* 	error_cmd(CMD_NAME, tar_name); */
/* 	return EXIT_FAILURE; */
/*       } */
/*     for(int i = 0; i < nb_of_files_in_tar; i++) */
/*       { */
/* 	char *c = NULL; */
/* 	char *d = NULL; */
/* 	char *the_file_to_list = malloc(strlen(name_in_tar)); */
/* 	cut_name(header[i].name, name_in_tar, the_file_to_list); */
/* 	//Conditions for the print of ls -l when we don't want list a file at the root */
/* 	if(strcmp(name_in_tar, header[i].name)!=0 */
/* 	   && ((d = strstr(header[i].name, name_in_tar)) != NULL) */
/* 	   && name_in_tar[strlen(name_in_tar)-1] == '/' */
/* 	   && name_in_tar[strlen(name_in_tar)] == '\0' */
/* 	   && ((c = strstr(the_file_to_list, "/")) == NULL || c[1] == '\0' )) */
/* 	  { */
/* 	    write_ls(header, header[i], the_file_to_list, nb_of_files_in_tar); */
/* 	  } */
/* 	free(the_file_to_list); */
/*       } */
/*   }else{//Conditions for the print of ls -l when we don't want to list a file at the root */
/*     for(int i = 0; i < nb_of_files_in_tar; i++) */
/*       { */
/* 	char *c = NULL; */
/* 	if((c = strstr(header[i].name, "/")) == NULL || c[1] == '\0' ) */
/* 	  { */
/* 	    write_ls(header, header[i], header[i].name, nb_of_files_in_tar); */
/* 	  } */
/*       } */
/*   } */
/*   free(header); */
/*   close(tar_fd); */
/*   return 0; */
/* } */

/* /\** */
/*  * Lists all files of a tar */
/*  *\/  */
/* int ls_(char *tar_name, char *name_in_tar) */
/* {   */
/*   int tar_fd = open(tar_name, O_RDONLY); */
/*   if (tar_fd < 0) */
/*     { */
/*       error_cmd(CMD_NAME, tar_name); */
/*       return -1; */
/*     } */
  
/*   int empty = 1; */

/*   if(is_file(header, name_in_tar, nb_of_files_in_tar, 0)) */
/*     { */
/*       close(tar_fd); */
/*       return 0; */
/*     } */
/*   if(*name_in_tar != '\0'){ */
/*     if(name_in_tar[strlen(name_in_tar)-1] != '/'){ strcat(name_in_tar, "/");} */
/*     if (tar_access(tar_name, name_in_tar, R_OK) == -1) */
/*       { */
/* 	name_in_tar[-1] = '/'; */
/* 	error_cmd(CMD_NAME, tar_name); */
/* 	return EXIT_FAILURE; */
/*       } */
/*     for(int i = 0; i < nb_of_files_in_tar; i++) */
/*       { */
/* 	char *c = NULL; */
/* 	char *d = NULL; */
/* 	char *the_file_to_list = malloc(strlen(name_in_tar)); */
/* 	cut_name(header[i].name, name_in_tar, the_file_to_list); */
/* 	//Conditions for the print of ls when we don't want to list a file at the root */
/* 	if(strcmp(name_in_tar, header[i].name)!=0 */
/* 	   && ((d = strstr(header[i].name, name_in_tar)) != NULL && name_in_tar[strlen(name_in_tar)-1] == '/' && name_in_tar[strlen(name_in_tar)] == '\0') */
/* 	   && ((c = strstr(the_file_to_list, "/")) == NULL || c[1] == '\0' )) { */
/* 	  write(STDOUT_FILENO, the_file_to_list, strlen(the_file_to_list)); */
/* 	  write(STDOUT_FILENO, "   ", 3); */
/* 	  empty = 0; */
/* 	} */
/* 	if(i == nb_of_files_in_tar-1 && !empty) */
/* 	  write(STDOUT_FILENO, "\n", 1); */
/* 	free(the_file_to_list); */
/*       } */
/*   } */
/*   else {//Conditions for the print of ls when we want to list a file at the root */
/*     for(int i = 0; i < nb_of_files_in_tar; i++) */
/*       { */
/* 	char *c = NULL; */
/* 	if((c = strstr(header[i].name, "/")) == NULL || c[1] == '\0' ){ */
/* 	  write(STDOUT_FILENO, header[i].name, strlen(header[i].name)); */
/* 	  write(STDOUT_FILENO, "   ", 3); */
/* 	  empty = 0; */
/* 	} */
/* 	if(i == nb_of_files_in_tar-1 && !empty) */
/* 	  write(STDOUT_FILENO, "\n", 1); */
/*       } */
/*   } */
/*   free(header); */
/*   close(tar_fd); */
/*   return 0; */
/* } */

int ls (char *tar_name, char *filename, char *options)
{
  int tar_fd;
  bool long_format;
  char *corrected_name;
  

  long_format = false;
  
  if (strchr(options, 'l'))
    long_format = true;

  
  // on vérifie que filename existe dans le tar
  corrected_name = get_corrected_name(tar_name, filename);

  // n'existe pas
  if (!corrected_name)
    {
      error_cmd(CMD_NAME, filename);
      return -1;
    }

  tar_fd = open(tar_name, O_RDONLY);
  if (tar_fd < 0)
    return -1;
  
  // un dossier
  if (*filename == '\0' || is_dir_name(corrected_name))
    {      
      array *files = tar_ls_dir(tar_fd, corrected_name, false);
      if (!files)
	{
	  error_cmd(CMD_NAME, filename);
	  return error_pt(&tar_fd, 1, errno);
	}
      
      // on affiche le tableau
      print_files(files, long_format);

      array_free(files, false);
    }
  // un fichier
  else
    {
      struct posix_header hd;
      seek_header (tar_fd, corrected_name, &hd);
      
      print_header (&hd, long_format, true);
    }

  close(tar_fd);
  
  return 0;
}

int main (int argc, char **argv)
{
  command cmd = {
    CMD_NAME,
    ls,
    1,
    1,
    SUPPORT_OPT
  };
  
  return handle(cmd, argc, argv);
}
