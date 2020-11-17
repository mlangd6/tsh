#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "path_lib.h"
#include "tar.h"

#include <stdio.h>

char *split_tar_abs_path(char *path)
{
  if (path == NULL || path[0] != '/')
    {
      return NULL;
    }

  char *chr = path+1;

  while (*chr)
    {
      if(*chr == '/')
	{
	  *chr = '\0';	        
	  if (is_tar(path) == 1)
	    {	
	      return chr + 1;
	    }
	  *chr = '/';
	}    
      chr++;
    }

  if (is_tar(path) == 1)
    return chr;
  
  return NULL;
}


/* Reduce an absolute path (i.e. a path starting with a / ) */
char *reduce_abs_path(const char *path, char *resolved)
{
  if (path == NULL || path[0] != '/')
    {
      errno = EINVAL;
      return NULL;
    }
  
  if (path[0] == '\0')
    {
      errno = ENOENT;
      return NULL;
    }
  
  char *ret, *dest, *ret_end, *in_tar;
  
  if (resolved == NULL)
    {
      ret = malloc(PATH_MAX);
      if (ret == NULL)
	return NULL;
    }
  else
    ret = resolved;

  ret[0] = '/';
  dest = ret + 1;
  ret_end = ret + PATH_MAX;
  in_tar = NULL;
  
  const char *name_start, *name_end;
  for (name_start = (name_end = path + 1); name_start[0] != '\0'; name_start = name_end)
    {
      // on saute les multiples /
      while (name_start[0] == '/')
	name_start++;

      // on cherche la fin du mot
      for (name_end = name_start; name_end[0] != '\0' && name_end[0] != '/'; name_end++)
	;
      
      
      if (name_end - name_start == 0) //on arrive à la fin de PATH
	break;
      else if (name_start[0] == '.' && name_end - name_start == 1) // ./
	;
      else if (name_start[0] == '.' && name_start[1] == '.' && name_end - name_start == 2) // ../
	{
	  if (dest > ret + 1) // si on n'est pas à la racine...
	    {
	      dest--;
	      while (dest[-1] != '/') // on fait revenir en arrière DEST
		dest--;
	    }
	}
      else
	{
	  int name_size = name_end - name_start;

	  if (name_end[0] == '/')
	    name_size++;
	  
	  if (dest[-1] != '/')
	    {
	      dest[0] = '/';
	      dest++;
	    }
	  
	  if (dest + name_size >= ret_end) // si on va dépasser la fin du buffer
	    {
	      if (resolved)
		{
		  errno = ENAMETOOLONG;
		  dest[0] = '\0'; // TODO: peut-être qu'on peut mieux corriger l'erreur
		  goto error;
		}
	      ptrdiff_t dest_offset = dest - ret;
	      size_t new_size = (ret_end - ret) + name_size;
	      ret = realloc (ret, new_size + 1);
	      dest = ret + dest_offset;
	    }

	  memmove (dest, name_start, name_size); // on copie le mot
	  dest += name_size; // on place dest à la fin du mot
	  dest[0] = '\0';

	  in_tar = split_tar_abs_path (ret);
	  
	  struct stat st;
	  if (stat(ret, &st) < 0)
	    goto error;

	  if (in_tar == NULL && !S_ISDIR (st.st_mode) && name_end[0] != '\0') // pas de tar en jeu
	    {
	      errno = ENOTDIR;
	      goto error;
	    }
	  else if (in_tar != NULL)
	    {
	      if (in_tar[0] != '\0')
		{
		  if (tar_access(ret, in_tar, F_OK) < 0)
		    goto error;
		  in_tar[-1] = '/';    
		}
	      else if (in_tar[-1] == '\0')
		in_tar[-1] = '/';
	    }
	}
    }
  
  /* if (dest > ret + 1 && dest[-1] == '/' && in_tar != NULL && in_tar[0] == '\0') */
  /*   --dest; */
  
  dest[0] = '\0';
  
  return ret;

 error:
  if (resolved == NULL)
    free(ret);
  return NULL;
}
