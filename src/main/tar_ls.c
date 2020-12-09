#include "tar.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "array.h"
#include "errors.h"
#include "utils.h"


/**
 * Lists all files passing a predicate in a tar.
 */
static array* tar_ls_if (int tar_fd, bool (*predicate)(const struct posix_header *))
{
  array *ret;
  ssize_t size_read;
  tar_file tf;

  lseek(tar_fd, 0, SEEK_SET);
  ret = array_create (sizeof(tar_file));
  tf.tar_fd = tar_fd;

  while ((size_read = read(tar_fd, &tf.header, BLOCKSIZE)))
    {
      if (size_read != BLOCKSIZE)
	{	  
	  array_free(ret, false);
	  return NULL;
	}

      if (tf.header.name[0] == '\0')
	break;
      
      if (predicate (&tf.header)) // on ajoute au tableau si le prédicat est vrai
	{	  
	  tf.file_start = lseek(tar_fd, 0, SEEK_CUR) - BLOCKSIZE;
	  
	  array_insert_last (ret, &tf);
	}
      
      skip_file_content(tar_fd, &tf.header);
    }
  
  return ret;
}


static bool always_true (const struct posix_header *header)
{
  return true;
}


/** 
 * Lists all files in a tar.
 *
 * The file descriptor `tar_fd` must be already opened with at least read mode.
 *
 * @param tar_fd the file descriptor referencing a tar
 * @return a pointer to an array of tar_file; `NULL` if there are errors
 */
array* tar_ls_all (int tar_fd)
{
  return tar_ls_if(tar_fd, always_true);
}


/** 
 * Lists all files from a directory contained in a tar.
 *
 * The file descriptor `tar_fd` must be already opened with at least read mode and `dir_name` must be a null terminated string.
 * To list files at root just put an empty string for `dir_name` otherwise make sure `dir_name` ends with a `/`.
 * 
 * Note that, the returned array has no entry for the listed directory (i.e. `dir_name`).
 *
 * @param tar_fd a file descriptor referencing a tar
 * @param dir_name the directory to list
 * @param rec if `true` then subdirectories are also listed
 * @return a pointer to an array of tar_file; `NULL` if there are errors
 */
array* tar_ls_dir (int tar_fd, const char *dir_name, bool rec)
{  
  bool in_dir(const struct posix_header *header)
  {
    if (is_prefix(dir_name, header->name) != 1)
      return false;

    if (rec)
      return true;
    
    char *c = strchr(header->name + strlen(dir_name), '/');
    return !c || !c[1]; // pas de '/' ou '/' à la fin
  }

  if (*dir_name == '\0')
    return tar_ls_if(tar_fd, in_dir);
    
  // on vérifie que dir_name est un dossier et qu'il existe bien
  if (!is_dir_name(dir_name) || ftar_access(tar_fd, dir_name, F_OK) == -1)
    return NULL;
  
  return tar_ls_if(tar_fd, in_dir);
}




/** 
 * List all files contained in a tar. 
 *
 * @param tar_name the tar to list
 * @param nb_headers an address to store the size of the returned array
 * @return on success, a malloc'd array of all headers; NULL on failure
 */
struct posix_header *tar_ls(const char *tar_name, int *nb_headers)
{
  int tar_fd = open(tar_name, O_RDONLY);
  if (tar_fd < 0)
    return error_p(&tar_fd, 1, errno);

  *nb_headers = nb_files_in_tar(tar_fd);
  if( *nb_headers < 0)
    return error_p(&tar_fd, 1, errno);
  ssize_t size_read;

  struct posix_header *list_header = malloc((*nb_headers) * sizeof(struct posix_header));
  assert(list_header);

  for(int i=0; i < *nb_headers; i++)
    {
      size_read = read(tar_fd, list_header+i, BLOCKSIZE);

      if(size_read != BLOCKSIZE)
	{
	  free(list_header);
	  return error_p(&tar_fd, 1, errno);
	}
      skip_file_content(tar_fd, list_header+i);
    }
  close(tar_fd);
  return list_header;
}
