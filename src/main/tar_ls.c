#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "array.h"
#include "errors.h"
#include "tar.h"
#include "utils.h"


char dir_name_glob[PATH_MAX];
bool in_dir_rec;

array* tar_ls_if (int tar_fd, bool (*predicate)(const struct posix_header *))
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


array* tar_ls_all (int tar_fd)
{
  return tar_ls_if(tar_fd, always_true);
}


static bool in_dir(const struct posix_header *header)
{
  if (is_prefix(dir_name_glob, header->name) != 1)
    return false;

  if (in_dir_rec)
    return true;

  char *c = strchr(header->name + strlen(dir_name_glob), '/');
  return !c || !c[1]; // pas de '/' ou '/' à la fin

}


array* tar_ls_dir (int tar_fd, const char *dir_name, bool rec)
{
  in_dir_rec = rec;
  strcpy(dir_name_glob, dir_name);
  if (*dir_name == '\0')
    return tar_ls_if(tar_fd, in_dir);

  // on vérifie que dir_name est un dossier et qu'il existe bien
  if (!is_dir_name(dir_name) || ftar_access(tar_fd, dir_name, F_OK) == -1)
    return NULL;

  return tar_ls_if(tar_fd, in_dir);
}



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
