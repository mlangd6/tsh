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


static int extract_reg_file (const tar_file *tf, int dest_fd)
{
  lseek(tf->tar_fd, tf->file_start + BLOCKSIZE, SEEK_SET);
  
  int fd = openat(dest_fd, tf->header.name, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
  if (fd < 0)
    return -1;
  
  size_t file_size = get_file_size(&tf->header);
  
  return read_write_buf_by_buf(tf->tar_fd, fd, file_size, BUFSIZE);
}


static int make_path(int dest_fd, char *path)
{  
  char *p = path;
  while ((p = strchr(p, '/')) && p[1])
    {      
      *p = '\0';

      if (mkdirat(dest_fd, path, 0777 & ~getumask()) == -1)
	{
	  if (errno != EEXIST)
	    {
	      *p = '/';
	      return -1;
	    }
	}
      
      *p = '/';
      p++;
    }  

  return 0;
}


/**
 * Extracts the content of a directory from a tar.
 *
 * `dest` must designate an already existing directory.
 * Files from `dir_name` are extracted in `dest/dir_name/`.
 *
 * @param tar_name the path to the tar
 * @param dir_name the directory to extract from the tar
 * @param dest the path to the output directory
 * @return on success 0; otherwise -1
 */
int tar_extract_dir(const char *tar_name, const char *dir_name, const char *dest)
{
  int tar_fd, dest_fd;
  array *arr;
  tar_file *tf;

  tf = NULL;
  
  tar_fd = open(tar_name, O_RDONLY);
  if (tar_fd < 0)    
    return -1;

  dest_fd = open(dest, O_DIRECTORY);
  if (dest_fd < 0)
    return error_pt(&tar_fd, 1, errno);
  
  arr = tar_ls_dir(tar_fd, dir_name, true);
  if (!arr)
    goto error;

  
  array_sort(arr, extract_order);

  
  // on extrait un par un les fichiers
  for (int i=0; i < array_size(arr); i++)
    {
      tf = array_get(arr, i);

      if (!tf)
	goto error;
      
      // on crée le chemin d'extraction si besoin
      if (make_path(dest_fd, tf->header.name) < 0)
	goto error;
      
      switch (tf->header.typeflag)
	{
	case AREGTYPE:
	case REGTYPE:
	  extract_reg_file(tf, dest_fd);
	  break;
	  
	case DIRTYPE:
	  mkdirat(dest_fd, tf->header.name, 0777 & ~getumask());
	  break;

	case SYMTYPE:
	  symlinkat(tf->header.linkname, dest_fd, tf->header.name);
	  break;

	case LNKTYPE:
	  linkat(AT_FDCWD, tf->header.linkname, dest_fd, tf->header.name, 0);
	  break;	  
	}

      free(tf);
    }

  array_free(arr, false);
  close(dest_fd);
  close(tar_fd);
  
  return 0;

 error:
  if (tar_fd >= 0)
    close(tar_fd);

  if (dest_fd >= 0)
    close(dest_fd);
  
  if (arr)
    array_free(arr, false);

  if (tf)
    free(tf);
  
  return -1;
}
  

/**
 * Copy the content of a file from a tar into a file descriptor.
 *
 * @param tar_name the tar in which we want to read `filename`
 * @param filename the file we want to read
 * @param fd a file descriptor to write to
 * @return 0 on success; -1 otherwise
 */
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
