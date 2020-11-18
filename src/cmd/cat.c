#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "errors.h"
#include "path_lib.h"
#include "tar.h"

#define CMD_NAME "cat"

int main(int argc, char *argv[])
{
  if (argc == 1)
    {
      execlp(CMD_NAME, CMD_NAME, NULL);
    }

  if (argv[1][0] == '-')
    {
      execvp(argv[0], argv);
      return EXIT_FAILURE;
    }

  char *in_tar;
  char path[PATH_MAX];
  
  for (int i = 1; i < argc; i++)
    {
      char *ret = reduce_abs_path (argv[i], path);

      if (ret == NULL)
	{
	  error_cmd(CMD_NAME, argv[i]);
	  continue;
	}
      
      in_tar = split_tar_abs_path(path);
      
      if (in_tar == NULL) // le chemin n'implique pas de tar
	{
	  int f = fork(), w;
	  switch(f)
	    {
	    case -1:
	      error_cmd(CMD_NAME, "fork");
	      return EXIT_FAILURE;
	    case 0: // son
	      execlp(CMD_NAME, CMD_NAME, path, NULL);
	      return EXIT_FAILURE;
	    default:
	      wait(&w);
	      return WEXITSTATUS(w);
	    }
	}
      else
	{
	  if (!(*in_tar))
	    {
	      errno = EISDIR;
	      error_cmd(CMD_NAME, argv[i]);
	    }
	  else if (tar_cp_file(path, in_tar, STDOUT_FILENO) < 0)
	    {
	      in_tar[-1] = '/';
	      error_cmd(CMD_NAME, argv[i]);
	    }
	}
    }
  
  return EXIT_SUCCESS;
}
