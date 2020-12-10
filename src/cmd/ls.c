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
#define SUPPORT_OPT "l"

/** Command name */
#define CMD_NAME "ls"

/* static int max_nlink_width; */
/* static int max_owner_width; */
/* static int max_group_width; */
/* static int max_size_width; */

/* static int total_block; */

static char *get_corrected_name (const char *tar_name, const char *filename); 
static char *get_last_component(char *path);


static int print_string(const char *string);

static int print_files (array *files, bool long_format);
static int print_header (struct posix_header *header, bool long_format, bool newline);

static int print_filetype(char typeflag);
static int print_filemode(char mode[8]);
static int print_filename(char name[100]);



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

  strcat(corrected_name, "/");
  
  // filename/ existe dans le tar
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
      print_filetype(header->typeflag);
      print_filemode(header->mode);
      print_string(" ");
      
      print_filename(header->name);
    }
  else
    {
      print_filename(header->name);
    }
  
  if (newline)
    print_string("\n");

  return 0;
}

static char *get_last_component(char *path)
{
  char *ret = strrchr(path, '/');

  // si on ne trouve pas de slash (i.e. un fichier à la racine)
  if (!ret)
    return path;

  // on est dans le cas : dir/fic
  if (ret[1] != '\0')
    return ret + 1;
    
  // on va cherche le début du mot
  ret--;
  for (; ret != path && *ret != '/'; ret--)
    ;

  return *ret == '/' ? ret + 1 : ret;
}

static int print_filename(char name[100])
{
  char *filename = get_last_component(name);
  return print_string(filename);
}

static int print_filetype(const char typeflag)
{
  switch (typeflag)
    {
    case DIRTYPE:
      return print_string("d");

    case SYMTYPE:
      return print_string("l");

    case FIFOTYPE:
      return print_string("p");

    default:
      return print_string("-");
    }

  return 0;
}

static int print_filemode(char mode[8])
{
  char str[10];
  int converted_mode;

  converted_mode = strtol(mode, NULL, 8);
  
  str[0] = converted_mode & TUREAD  ? 'r' : '-';
  str[1] = converted_mode & TUWRITE ? 'w' : '-';
  str[2] = converted_mode & TUEXEC  ? 'x' : '-';

  str[3] = converted_mode & TGREAD  ? 'r' : '-';
  str[4] = converted_mode & TGWRITE ? 'w' : '-';
  str[5] = converted_mode & TGEXEC  ? 'x' : '-';

  str[6] = converted_mode & TOREAD  ? 'r' : '-';
  str[7] = converted_mode & TOWRITE ? 'w' : '-';
  str[8] = converted_mode & TOEXEC  ? 'x' : '-';

  str[9] = '\0';  

  return print_string(str);
}

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

  free(corrected_name);
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
