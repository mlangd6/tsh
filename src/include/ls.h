#ifndef LS_H
#define LS_H

/* Representing the command ls -l which shows the type of the files, rights,
   the number of links the user name, group name, size, last modification
   date, and the file name. Take a char * in parameter representing the
   file .tar that we want to list */
int ls_l(const char *tar_name);

/* Representing the command ls, list the files of a tarball. */
int ls(const char *tar_name);

#endif
