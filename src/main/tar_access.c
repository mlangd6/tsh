#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

#include "errors.h"
#include "tar.h"
#include "utils.h"


/* Get the type of user according to HD,
   Returns:
   0 if current user is the user in hd
   1 if current groupe is the same of hd
   2 else
*/
static int type_of_user(struct posix_header *hd)
{
  struct passwd *pwd = getpwuid(getuid());
  int uid = strtol(hd -> uid, NULL, 8);
  if (uid == pwd -> pw_uid)
    return 0;
  int gid =strtol(hd -> gid, NULL, 8);
  if (gid == pwd -> pw_uid)
    return 1;
  return 2;
}

static int is_mode_correct(int mode)
{
  return mode == F_OK;
}


/* Check user's permissions for file FILE_NAME in tar at path TAR_NAME */
int tar_access(const char *tar_name, const char *file_name, int mode)
{
  if(!is_mode_correct(mode))
    {
      errno = EINVAL;
      return -1;
    }

  int tar_fd = open(tar_name, O_RDONLY);
  if(tar_fd < 0)
    return -1;
  size_t nb_headers = nb_files_in_tar(tar_fd);
  close(tar_fd);


  struct posix_header *headers = tar_ls(tar_name);
  if( !headers )
    return -1;

  int found  = 0;
  int is_dir = is_dir_name(file_name);

  for(int i=0; i < nb_headers && found != 1; i++)
    {
      if(!strcmp(headers[i].name, file_name)) // fichier exactement trouvé
	found = 1;
      else if(is_dir && is_prefix(file_name, headers[i].name)) // dossier existant à travers ses sous-fichiers
	found = 2;
    }

  free(headers);

  if(!found)
    {
      errno = ENOENT;
      return -1;
    }

  return found;
}
