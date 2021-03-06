#include "tar.h"

#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <linux/limits.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "array.h"
#include "errors.h"
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
  free(groups);
  if ( (mode & R_OK && !(R_OK & rights[type_u]))
       ||   (mode & W_OK && !(W_OK & rights[type_u]))
       ||   (mode & X_OK && !(X_OK & rights[type_u])) )
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
static int simple_tar_access(const char *filename, array *headers, struct passwd *pwd, int mode)
{
  int r;
  int found = 0;
  int index = -1;
  int is_dir = is_dir_name(filename);
  tar_file *tf = NULL;

  for (int i = 0; i < array_size(headers); i++)
    {
      tf = array_get(headers, i);

      if (!strcmp(tf->header.name, filename))
	{
	  found = 1;
	  index = i;
	}
      else if (is_dir && is_prefix(filename, tf->header.name) && found != 1)
	{
	  found = 2;
	}

      free(tf);
    }

  if (!found)
    {
      errno = ENOENT;
      return -1;
    }
  else if (mode == F_OK || found == 2)
    {
      return found;
    }
  // else found == 1

  tf = array_get(headers, index);
  r = has_rights(tf->header, pwd, mode);
  free(tf);

  return r == 0 ? 1 : -1;
}

/* Check user's permissions for every parent directory of FILENAME and FILENAME itself */
static int tar_access_all(const char *filename, array *headers, struct passwd *pwd, int mode)
{
  size_t filename_len = strlen(filename);
  char *cpy = malloc(filename_len + 1);
  memmove(cpy, filename, filename_len + 1);
  char *it = cpy;
  char tmp;
  while ((it = strchr(it, '/')) != NULL && it[1] != '\0')
  {
    tmp = it[1];
    it[1] = '\0';
    if (simple_tar_access(cpy, headers, pwd, X_OK) == -1) // Test if parent dir is executable
    {
      it[1] = tmp;
      free(cpy);
      return -1;
    }
    it[1] = tmp;
    it++;
  }
  int res = simple_tar_access(cpy, headers, pwd, mode);
  free(cpy);
  return res;
}

/* Check user's permissions for file FILE_NAME in tar at path TAR_NAME */
int tar_access(const char *tar_name, const char *file_name, int mode)
{
  int tar_fd = open(tar_name, O_RDONLY);
  if (tar_fd <0)
    return -1;

  int r = ftar_access(tar_fd, file_name, mode);

  close(tar_fd);
  return r;
}

/* identical to tar_access except that the tar about which information is to be retrieved is specified by the file descriptor tar_fd.*/
int ftar_access(int tar_fd, const char *file_name, int mode)
{
  if (!is_mode_correct(mode))
    {
      errno = EINVAL;
      return -1;
    }

  array *headers = tar_ls_all(tar_fd);
  if (!headers)
    return -1;

  struct passwd *pwd = getpwuid(getuid());
  int found;

  if (pwd -> pw_uid == 0)
    found = simple_tar_access(file_name, headers, pwd, F_OK);
  else
    found = tar_access_all(file_name, headers, pwd, mode);

  array_free(headers, false);

  return found;
}
