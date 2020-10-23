#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "errors.h"
#include "tar.h"
#include "utils.h"

/* Delete all files starting with DIRNAME i.e. delete directory DIRNAME in the tar referenced by TAR_FD.
   Last character of DIRNAME is '/'
   Returns :
   0  if it succeed
   -1 if a system call failed */
static int tar_rm_dir(int tar_fd, const char *dirname)
{
  unsigned int file_size;
  struct posix_header file_header;  
  ssize_t size_read;
  off_t file_start, file_end, tar_end;

  tar_end = lseek(tar_fd, 0, SEEK_END);
  
  lseek(tar_fd, 0, SEEK_SET);
      
  while((size_read = read(tar_fd, &file_header, BLOCKSIZE)) > 0)
    {
      if(size_read != BLOCKSIZE)
	{
	  return -1;
	}
      if(file_header.name[0] == '\0')
	{
	  ftruncate(tar_fd, tar_end);
	  return 0;
	}
      else if(is_prefix(dirname, file_header.name))
	{
	  file_size  = get_file_size(&file_header);
	  file_start = lseek(tar_fd, -BLOCKSIZE, SEEK_CUR); // on était à la fin d'un header, on se place donc au début
	  file_end   = file_start + BLOCKSIZE + number_of_block(file_size)*BLOCKSIZE;

	  if( fmemmove(tar_fd, file_end, tar_end - file_end, file_start) < 0) // on décale le contenu
	    return -1;

	  tar_end -= file_end - file_start;    // on réduit virtuellement la taille
	}
      else
	{
	  skip_file_content(tar_fd, &file_header);
	}	    
    }

  return -1; 
}


/* Delete FILENAME in the tar referenced by TAR_FD.
   FILENAME is supposed to be a regular file.
   Returns :
   0  if it succeed
   -1 if a system call failed
   -2 if FILENAME is not in the tar or FILENAME is a directory not finishing with '/' */
static int tar_rm_file(int tar_fd, const char *filename)
{
  unsigned int file_size;
  struct posix_header file_header;
  int r = seek_header(tar_fd, filename, &file_header);
  
  if(r < 0) // erreur
    {
      return -1;
    }
  else if( r == 0 || (file_header.typeflag == DIRTYPE) ) // Pas trouvé OU un dossier
    {
      return -2;
    }
  

  file_size = get_file_size(&file_header);
  
  off_t file_start = lseek(tar_fd, -BLOCKSIZE, SEEK_CUR), // on était à la fin d'un header, on se place donc au début
        file_end   = file_start + BLOCKSIZE + number_of_block(file_size)*BLOCKSIZE, 
        tar_end    = lseek(tar_fd, 0, SEEK_END);

  if(fmemmove(tar_fd, file_end, tar_end - file_end, file_start) < 0)
    return -1;

  ftruncate(tar_fd, tar_end - (file_end - file_start));
  return 0;
}


/* Open the tarball at path TAR_NAME and delete FILENAME if possible */
int tar_rm(const char *tar_name, const char *filename)
{
  int tar_fd = open(tar_name, O_RDWR);

  if (tar_fd < 0)
    return error_pt(tar_name, &tar_fd, 1);

  int r;

  if(is_dir_name(filename))
    {
      r = tar_rm_dir(tar_fd, filename);
    }
  else
    {
      r = tar_rm_file(tar_fd, filename);
    }
  
  if(r == -1) // erreur appel système
    return error_pt(tar_name, &tar_fd, 1);
  
  close(tar_fd);
  return r;
}
