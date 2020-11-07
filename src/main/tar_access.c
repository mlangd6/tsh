#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <linux/limits.h>

#include "errors.h"
#include "tar.h"
#include "utils.h"

static int is_mode_correct(int mode)
{
  return mode == F_OK || mode & R_OK || mode & W_OK || mode & X_OK;
}


/* Get the type of user according to HD,
   Returns:
   0 if current user is the user in hd
   1 if current groupe is the same of hd
   2 else
*/
static int type_of_user(struct posix_header hd, struct passwd *pwd, gid_t *groups, int nb_groups)
{

  uid_t uid = strtol(hd.uid, NULL, 8);
  if (uid == pwd -> pw_uid)
    return 0;
  gid_t gid =strtol(hd.gid, NULL, 8);
  for (int i = 0; i < nb_groups; i++)
  {
    if (gid == groups[i])
      return 1;
  }
  /* Test current gid because: it is unspecified whether the effective group ID
     of the calling process is included in the returned list (getgroups man page)*/
  if (pwd -> pw_gid == groups[1])
    return 1;
  return 2;
}
/* Returns 0 if current user has the rights that are in MODE in the header HD */
static int has_rights(struct posix_header hd, struct passwd *pwd, int mode)
{
  int nb_groups = getgroups(0, NULL);
  gid_t *groups = malloc(nb_groups * sizeof(gid_t));
  getgroups(nb_groups, groups);

  int type_u = type_of_user(hd, pwd, groups, nb_groups);
  int rights[] =
  {
    hd.mode[4] - '0',
    hd.mode[5] - '0',
    hd.mode[6] - '0'
  };

  if ( (mode & R_OK && !(4 & rights[type_u]))
  ||   (mode & W_OK && !(2 & rights[type_u]))
  ||   (mode & X_OK && !(1 & rights[type_u])) )
  {
    errno = EACCES;
    return -1;
  }
  return 0;
}

/* Try to access file inside tar without trying to access to parent directory
   Returns -1 if file is not found or has not the rights in mode
   1 if file was found and has the rights
   2 if file is a dir and has beeen found in his subfile
*/
static int simple_tar_access(const char *filename, struct posix_header *hds, int nb_hds, struct passwd *pwd, int mode)
{
  int found = 0;
  int index = -1;
  int is_dir = is_dir_name(filename);
  for (int i = 0; i < nb_hds; i++)
  {
    if (! strcmp(hds[i].name, filename))
    {
      found = 1;
      index = i;
    }
    else if (is_dir && is_prefix(filename, hds[i].name) && found != 1)
      found = 2;
  }
  if (!found)
  {
    errno = ENOENT;
    return -1;
  }
  else if (mode == F_OK || found == 2)
    return found;
  // else found == 1
  return (has_rights(hds[index], pwd, mode) == 0) ? 1 : -1;

}

/* Check user's permissions for every parent directory of FILENAME and FILENAME itself */
static int tar_access_all(const char *filename, struct posix_header *hds, int nb_hds, struct passwd *pwd, int mode)
{
  size_t filename_len = strlen(filename);
  char *cpy = malloc(filename_len + 1);
  memmove(cpy, filename, filename_len + 1);
  char *it = cpy;
  char tmp;
  while ((it = strchr(it, '/')) != NULL)
  {
    tmp = it[1];
    it[1] = '\0';
    if (simple_tar_access(cpy, hds, nb_hds, pwd, mode) == -1)
    {
      it[1] = tmp;
      free(cpy);
      return -1;
    }
    it[1] = tmp;
  }
  return simple_tar_access(cpy, hds, nb_hds, pwd, mode);
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
