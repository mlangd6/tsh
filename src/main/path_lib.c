#include <string.h>
#include <stdlib.h>

#include "tar.h"
#include "utils.h"

#include <stdio.h>

char *split_tar_abs_path(char *path) {
  if (path[0] != '/') {
    return NULL;
  }
  char *chr = path+1;

  while ( (chr = strchr(chr, '/')) != NULL) {
    *chr = '\0';
    if (is_tar(path) == 1) {
      return chr+1;
    }
    *chr = '/';
    chr++;
  }
  return strchr(path, '\0');
}

typedef enum COMPONENT_TYPE COMPONENT_TYPE;
enum COMPONENT_TYPE
  {
    DOT_SLASH,     // ./
    DOT_END,       // .\0
    DOT_DOT_SLASH, // ../
    DOT_DOT_END,   // ..\0
    SLASH,         // /
    NORMAL         // other cases
  };


static COMPONENT_TYPE get_first_component_type(const char *s)
{
  if( s[0] == '/' )
    return SLASH;

  if (is_prefix(".", s))
    {
      if (s[1] == '\0')
	return DOT_END;
      else if (s[1] == '/')
	return DOT_SLASH;
    }

  if (is_prefix("..", s))
    {
      if (s[2] == '\0')
	return DOT_DOT_END;
      else if (s[2] == '/')
	return DOT_DOT_SLASH;
    }
  
  return NORMAL;
}


void strmove(char *dest, char *src)
{
  memmove(dest, src, strlen(src)+1);
}

/* Reduce an absolute path (i.e. a path starting with a / ) */
char *reduce_abs_path(char *path)
{
  char **prev_chr = malloc(strlen(path) * sizeof(char *)); // Maximum number of '/' //TODO: Use a stack
  char *chr = path;
  int i = 0;
  
  while ((chr = strchr(chr, '/')) != NULL)
    {
      switch (get_first_component_type(++chr))
	{
	case DOT_SLASH:
	  strmove(chr, chr+2);
	  chr--;
	  break;
	case DOT_END:      
	  chr[0] = '\0';
	  break;
	case SLASH:	  
	  strmove(chr, chr+1);
	  chr--;
	  break;
	case NORMAL:
	  prev_chr[i++] = chr;
	  break;
	case DOT_DOT_END:
	  if (i == 0) // /..
	    {
	      chr[0] = '\0';
	    }
	  else
	    {
	      prev_chr[--i][0] = '\0';
	    }
	  break;
	case DOT_DOT_SLASH:
	  if (i == 0)
	    {
	      strmove(chr, chr+3);
	      chr--;
	    }
	  else
	    {
	      strmove(prev_chr[--i], chr + 3);
	      chr = prev_chr[i] - 1;	      
	    }
	  break;
	}
    }
  
  free(prev_chr);

  // on supprimer le / de fin s'il y en a un
  size_t len = strlen(path);  
  if (len != 1 && path[len - 1] == '/')
    {
      path[len-1] = '\0';
    }
  
  return path;
}
