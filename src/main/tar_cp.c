#include "tar.h"

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
#include "utils.h"


#define BUFSIZE BLOCKSIZE


static int make_path(int dest_fd, char *path);

static int extract_order(const void *lhs, const void *rhs);

static int extract_tar_file (const tar_file *tf, const char *extract_name, int dest_fd);
static int extract_reg_file (const tar_file *tf, const char *extract_name, int dest_fd);
static int ftar_extract_dir (int tar_fd, const char *full_path, const char *wanted_dir, int dest_fd);
static int ftar_extract_file (int tar_fd, const char *full_path, const char *wanted_file, int dest_fd);

/**
 * Create a path from a directory
 */
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

static int extract_tar_file (const tar_file *tf, const char *extract_name, int dest_fd)
{
  switch (tf->header.typeflag)
    {
    case AREGTYPE:
    case REGTYPE:
      return extract_reg_file (tf, extract_name, dest_fd);

    case DIRTYPE:
      return mkdirat (dest_fd, extract_name, 0777 & ~getumask());
      
    case SYMTYPE:
      return symlinkat (tf->header.linkname, dest_fd, extract_name);

    case LNKTYPE:
      return linkat (dest_fd, tf->header.linkname, dest_fd, extract_name, 0);

    default:
      return 0;
    }
}

/**
 * Extract a regular file from a tar
 */
static int extract_reg_file (const tar_file *tf, const char *extract_name, int dest_fd)
{
  lseek(tf->tar_fd, tf->file_start + BLOCKSIZE, SEEK_SET);
  
  int fd = openat(dest_fd, extract_name, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
  if (fd < 0)
    return -1;

  size_t file_size = get_file_size(&tf->header);

  return read_write_buf_by_buf(tf->tar_fd, fd, file_size, BUFSIZE);
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
static int ftar_extract_dir (int tar_fd, const char *full_path, const char *wanted_dir, int dest_fd)
{
  array *arr;
  tar_file *tf;
  char *extract_name;
  size_t wanted_dir_start;
  
  tf = NULL;
  
  arr = tar_ls_dir(tar_fd, full_path, true);
  if (!arr)
    goto error;

  wanted_dir_start = wanted_dir - full_path;
  array_sort(arr, extract_order);
  
  // on extrait un par un les fichiers
  for (int i=0; i < array_size(arr); i++)
    {
      tf = array_get(arr, i);
      
      if (!tf)
	goto error;

      extract_name = tf->header.name + wanted_dir_start;

      // on crée le chemin d'extraction si besoin
      if (make_path(dest_fd, tf->header.name) < 0)
	goto error;

      if (extract_tar_file (tf, extract_name, dest_fd) < 0)
	goto error;
      
      free(tf);
    }

  array_free(arr, false);

  return 0;

 error:
  if (arr)
    array_free(arr, false);

  if (tf)
    free(tf);

  return -1;
}

static int ftar_extract_file (int tar_fd, const char *full_path, const char *wanted_file, int dest_fd)
{
  tar_file tf;
  struct posix_header file_header;
  int r = seek_header(tar_fd, full_path, &file_header);

  if (r < 0)
    return -1;

  tf.tar_fd = tar_fd;
  tf.header = file_header;
  tf.file_start = lseek (tar_fd, -BLOCKSIZE, SEEK_CUR);    
  
  return extract_tar_file (&tf, wanted_file, dest_fd);
}


static const char *get_last_component(const char *path)
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


int tar_extract (const char *tar_name, const char *filename, const char *dest)
{
  int tar_fd, dest_fd, ret;
  const char *wanted_file;
  
  tar_fd = open(tar_name, O_RDONLY);
  if (tar_fd < 0)
    return -1;

  // on vérifie les droits en lecture... incomplet pour les dossiers
  if (ftar_access (tar_fd, filename, R_OK) < 0)
    return error_pt (&tar_fd, 1, errno);

  lseek(tar_fd, 0, SEEK_SET);
  
  dest_fd = open(dest, O_DIRECTORY);
  if (dest_fd < 0)
    return error_pt(&tar_fd, 1, errno);

  wanted_file = get_last_component (filename);
  
  if (is_empty_string(filename) || is_dir_name(filename))
    {
      ret = ftar_extract_dir (tar_fd, filename, wanted_file, dest_fd);
    }
  else
    {
      ret = ftar_extract_file (tar_fd, filename, wanted_file, dest_fd);
    }
  
  close(dest_fd);
  close(tar_fd);

  return ret;
}

/**
 * Copy the content of a file from a tar into a file descriptor.
 * filename should be readable
 * @param tar_name the tar in which we want to read `filename`
 * @param filename the file we want to read
 * @param fd a file descriptor to write to
 * @return 0 on success; -1 otherwise
 */
int tar_cp_file(const char *tar_name, const char *filename, int fd)
{
  if (tar_access (tar_name, filename, R_OK) < 0)
    return -1;
    
  int tar_fd = open(tar_name, O_RDONLY);

  if (tar_fd < 0)
    return -1;
    
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
