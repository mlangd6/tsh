#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "array.h"
#include "errors.h"
#include "tar.h"
#include "utils.h"

#define BUFSIZE BLOCKSIZE


static int extract_order(const void *lhs, const void *rhs)
{
  struct posix_header lhd = ((tar_file*)lhs)->header;
  struct posix_header rhd = ((tar_file*)rhs)->header;


  // les liens symboliques en dernier...
  if (lhd.typeflag == SYMTYPE)
    return 1;

  if (rhd.typeflag == SYMTYPE)
    return -1;

  // ... précédés des liens physiques
  if (lhd.typeflag == LNKTYPE)
    return 1;

  if (rhd.typeflag == LNKTYPE)
    return -1;

  // sinon on utilise ordre lexicographique
  return strcmp(lhd.name, rhd.name);
}


static int extract_reg_file (const tar_file *tf, const char *extract_path)
{
  lseek(tf->tar_fd, tf->file_start + BLOCKSIZE, SEEK_SET);
  
  int fd = open(extract_path, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
  if (fd < 0)
    return -1;
  
  size_t file_size = get_file_size(&tf->header);
  
  return read_write_buf_by_buf(tf->tar_fd, fd, file_size, BUFSIZE);
}


static int make_path(const char* dest, char *path)
{
  int dest_fd = open(dest, O_DIRECTORY);
  if (dest_fd < 0)
    return -1;
  
  char *p = path;
  while ((p = strchr(p, '/')) && p[1])
    {      
      *p = '\0';

      if (mkdirat(dest_fd, path, 0777 & ~getumask()) == -1)
	{
	  if (errno != EEXIST)
	    {
	      *p = '/';
	      close(dest_fd);
	      return -1;
	    }
	}
      
      *p = '/';
      p++;
    }  

  close(dest_fd);
  return 0;
}

/**
 * Extracts the content of a directory from a tar.
 *
 * @param tar_name the path to the tar
 * @param dir_name the directory to extract from #tar_name
 * @param dest the path to the output directory
 * @return On success 0; otherwise -1
 */
int tar_extract_dir(const char *tar_name, const char *dir_name, const char *dest)
{
  int tar_fd;
  array *arr;
  char *extract_path, *extract_path_end;
  tar_file *tf;

  tf = NULL;
  extract_path = NULL;
  
  tar_fd = open(tar_name, O_RDONLY);
  if (tar_fd < 0)    
    return error_pt(&tar_fd, 1, errno);
  
  arr = tar_ls_dir(tar_fd, dir_name, true);
  if (!arr)
    goto error;

  
  array_sort(arr, extract_order);
  
  size_t dest_len = strlen(dest);
  if (dest_len == 0)
    goto error;
  
  extract_path = malloc(dest_len + 102) ; // name[100] + 1 (\0 à la fin) + 1 (le / du milieu si besoin) = 102
  if (!extract_path)
    goto error;

  strcpy(extract_path, dest);
  extract_path_end = extract_path + dest_len;

  // on doit vérifier si le dossier de destination se termine par un /
  if (extract_path_end[-1] != '/')
    {
      *extract_path_end = '/';
      *(++extract_path_end) = '\0';
    }


  // on extrait un par un les fichiers
  for (int i=0; i < array_size(arr); i++)
    {
      tf = array_get(arr, i);

      if (!tf)
	goto error;
      
      // on construit le chemin complet d'extraction
      strcpy(extract_path_end, tf->header.name);

      // on crée le chemin d'extraction si besoin
      if (make_path(dest, tf->header.name) < 0)
	goto error;
      
      switch (tf->header.typeflag)
	{
	case AREGTYPE:
	case REGTYPE:
	  extract_reg_file(tf, extract_path);
	  break;
	  
	case DIRTYPE:
	  mkdir(extract_path, 0777 & ~getumask());
	  break;

	case SYMTYPE:
	  symlink(tf->header.linkname, extract_path);
	  break;

	case LNKTYPE:
	  link(tf->header.linkname, extract_path);
	  break;	  
	}

      free(tf);
    }

  array_free(arr, false);
  close(tar_fd);
  
  return 0;

 error:
  if (tar_fd >= 0)
    close(tar_fd);

  if (arr)
    array_free(arr, false);

  if (extract_path)
    free(extract_path);

  if (tf)
    free(tf);
  
  return -1;
}
  

  

/* Open the tarball TAR_NAME and copy the content of FILENAME into FD */
int tar_cp_file(const char *tar_name, const char *filename, int fd)
{
  int tar_fd = open(tar_name, O_RDONLY);

  if (tar_fd < 0)
    return error_pt(&tar_fd, 1, errno);

  unsigned int file_size;
  struct posix_header file_header;
  int r = seek_header(tar_fd, filename, &file_header);

  if (r < 0) // erreur
    {
      return error_pt(&tar_fd, 1, errno);
    }
  else if (r == 0)
    {
      return error_pt(&tar_fd, 1, ENOENT);
    }
  else if (file_header.typeflag == DIRTYPE)
    {
      return error_pt(&tar_fd, 1, EISDIR);
    }
  else if (file_header.typeflag != AREGTYPE && file_header.typeflag != REGTYPE)
    {
      return error_pt(&tar_fd, 1, EPERM);
    }

  file_size = get_file_size(&file_header);
  if (read_write_buf_by_buf(tar_fd, fd, file_size, BUFSIZE) < 0)
    return error_pt(&tar_fd, 1, errno);

  close(tar_fd);

  return 0;
}
