#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

struct tar_fileinfo
{
  struct posix_header header;

  unsigned int nb_links;
};


static int max_nlink_width;
static int max_owner_width;
static int max_group_width;
static int max_size_width;

static int total_block;


/* utils */
static char* get_corrected_name (const char *tar_name, const char *filename); 
static char* get_last_component (char *path);

static int nb_of_digits (unsigned int n);

static int print_string (const char *string);
static int print_unsigned_int (unsigned int n);


static void init_ls ();
static void add_header_to_files (array *files, struct posix_header *hd);

static void update_files (array *files, int tar_fd);
static int count_nb_link (array *all, struct tar_fileinfo *info);
static bool is_in_dir(const char *dir_name, const char *filename);
static void update_widths (struct tar_fileinfo *info);
static void update_total_block (struct tar_fileinfo *info);

/* printing */
static void print_padding (int n);

static void print_total_block ();
static int print_files (array *files, bool long_format);
static int print_fileinfo (struct tar_fileinfo *tfi, bool long_format, bool newline);

static int print_filetype (char typeflag);
static int print_filemode (char mode[8]);
static void print_nb_links (unsigned int nb_links);
static void print_uname (char uname[32]);
static void print_gname (char gname[32]);
static void print_size (char size[12]);
static void print_mtime (char mtime[12]);
static int print_filename (char name[100]);





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


static int nb_of_digits (unsigned int n)
{
  int nb = 1;

  while (n > 9)
    {
      n /= 10;
      nb++;
    }
  
  return nb;
}


static int print_unsigned_int(unsigned int n)
{
  char buf[20];
  snprintf(buf, sizeof(buf), "%u", n);
  return print_string(buf);
}

static int print_string(const char *string)
{
  return write(STDOUT_FILENO, string, strlen(string)+1);
}



static void init_ls ()
{
  max_nlink_width = 0;
  max_nlink_width = 0;
  max_owner_width = 0;
  max_group_width = 0;
  max_size_width = 0;
   
  total_block = 0;
}

static void add_header_to_files (array *files, struct posix_header *hd)
{
  struct tar_fileinfo tfi = { *hd, 0 };
  array_insert_last (files, &tfi);
}



static void update_files (array *files, int tar_fd)
{
  array *all = tar_ls_all (tar_fd);

  struct tar_fileinfo *tfi;
  struct tar_fileinfo updated_tfi;
  
  for (int i=0; i < array_size (files); i++)
    {
      tfi = array_get (files, i);
      
      updated_tfi.header = tfi->header;
      updated_tfi.nb_links = count_nb_link (all, &updated_tfi);

      free (array_set (files, i, &updated_tfi));

      update_widths (&updated_tfi);
      update_total_block (&updated_tfi);
      
      free (tfi);
    }

  array_free (all, false);
}

static int count_nb_link (array *all, struct tar_fileinfo *info)
{
  int nb;
  tar_file *tf;

  nb = 1;  
  
  for (int i = 0; i < array_size (all); i++)
    {
      tf = array_get (all, i);
      
      if (!strcmp(tf->header.linkname, info->header.name))
	{
	  nb++;
	}
      else if (info->header.typeflag == DIRTYPE && is_in_dir(info->header.name, tf->header.name))
	{
	  nb++;
	}
      
      free (tf);
    }
  
  return nb;
}

static bool is_in_dir(const char *dir_name, const char *filename)
{
  if (is_prefix(dir_name, filename) != 1)
    return false;
    
  char *c = strchr(filename + strlen(dir_name), '/');
  return !c || !c[1]; // pas de '/' ou 1er '/' à la fin
}

static void update_widths (struct tar_fileinfo *info)
{
  unsigned int n;
  
  n = nb_of_digits (info->nb_links);
  if (max_nlink_width < n)
    max_nlink_width = n;

  n = nb_of_digits (get_file_size(&info->header));
  if (max_size_width < n)
    max_size_width = n;

  n = strlen (info->header.uname);
  if (max_owner_width < n)
    max_owner_width = n;

  n = strlen (info->header.gname);
  if (max_group_width < n)
    max_group_width = n;  
}

static void update_total_block (struct tar_fileinfo *info)
{
  unsigned int size = get_file_size (&info->header);

  total_block += number_of_block(size);
}



static void print_padding (int n)
{
  for (int i=0; i < n; i++)
    print_string (" ");
}

static void print_total_block ()
{
  print_string("total ");
  print_unsigned_int (total_block);
  print_string("\n");
}

static int print_files (array *files, bool long_format)
{
  if (array_size(files) == 0)
    return 0;
  
  struct tar_fileinfo *tfi;
  int i;

  if (long_format)
    print_total_block ();
  
  for (i=0; i < array_size(files) - 1; i++)
    {
      tfi = array_get(files, i);
      
      if (long_format)
	{
	  print_fileinfo (tfi, long_format, true);
	}
      else
	{
	  print_fileinfo (tfi, long_format, false);
	  print_string("  ");
	}
      
      free(tfi);
    }

  tfi = array_get(files, i);
  print_fileinfo (tfi, long_format, true);
  free(tfi);
  
  return 0;
}

static int print_fileinfo (struct tar_fileinfo *tfi, bool long_format, bool newline)
{
  if (long_format)
    {      
      print_filetype (tfi->header.typeflag);
      print_filemode (tfi->header.mode);

      print_string (" ");

      print_nb_links (tfi->nb_links);

      print_string (" ");

      print_uname (tfi->header.uname);

      print_string (" ");

      print_gname (tfi->header.gname);

      print_string (" ");

      print_size (tfi->header.size);

      print_string (" ");

      print_mtime(tfi->header.mtime);

      print_string (" ");
	
      print_filename(tfi->header.name);

      if (tfi->header.typeflag == SYMTYPE)
	{
	  print_string (" -> ");
	  print_filename(tfi->header.linkname);
	}
    }
  else
    {
      print_filename(tfi->header.name);
    }
  
  if (newline)
    print_string("\n");

  return 0;
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

static void print_nb_links(unsigned int nb_links)
{
  print_padding (max_nlink_width - nb_of_digits(nb_links));
  print_unsigned_int (nb_links);
}

static void print_uname(char uname[32])
{
  print_padding (max_owner_width - strlen(uname));  
  print_string (uname);
}

static void print_gname(char gname[32])
{
  print_padding (max_group_width - strlen(gname));  
  print_string (gname);
}

static void print_size(char size[12])
{
  unsigned int file_size = 0;
  sscanf(size, "%o", &file_size);

  print_padding (max_size_width - nb_of_digits(file_size));
  print_unsigned_int (file_size);
}

static void print_mtime(char mtime[12])
{
  int t;
  sscanf(mtime, "%o", &t);
  time_t timestamp = t;
  struct tm *realtime = gmtime(&timestamp);

  char buffer[1000]; // un peu arbitraire...
  strftime(buffer, 1000, "%d %b.  %H:%M", realtime);

  print_string(buffer);
}

static int print_filename(char name[100])
{
  char *filename = get_last_component(name);
  return print_string(filename);
}

/**
 * `ls` command
 */
int ls (char *tar_name, char *filename, char *options)
{
  int tar_fd;
  bool long_format;
  char *corrected_name;
  array *files; // on ajoute dans ce tableau les fichiers à afficher
      
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

  
  // On peut enfin initialiser
  long_format = false;
  
  if (strchr(options, 'l'))
    long_format = true;
  
  init_ls ();
  files = array_create(sizeof(struct tar_fileinfo));
  
  
  // un dossier
  if (*corrected_name == '\0' || is_dir_name(corrected_name))
    {      
      array *arr = tar_ls_dir(tar_fd, corrected_name, false);
      if (!arr)
	{
	  error_cmd(CMD_NAME, filename);
	  return error_pt(&tar_fd, 1, errno);
	}

      // on ajoute les en-têtes à files
      tar_file *tf;
      for (int i=0; i < array_size (arr); i++)
	{
	  tf = array_get(arr, i);
	  add_header_to_files (files, &tf->header);
	  free (tf);
	}
      
      array_free(arr, false);
    }
  // un fichier
  else
    {
      struct posix_header hd;
      seek_header (tar_fd, corrected_name, &hd);

      add_header_to_files (files, &hd);
    }
  
  // On peut enfin afficher
  update_files (files, tar_fd);
  print_files (files, long_format);


  // On fait le ménage
  array_free (files, false);
  free (corrected_name);
  close (tar_fd);
  
  return 0;
}

int main(int argc, char **argv) {
  unary_command cmd = {
    CMD_NAME,
    ls,
    true,
    true,
    SUPPORT_OPT
  };
  return handle_unary_command (cmd, argc, argv);
}
