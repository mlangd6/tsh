/**
  * @file remove.h
  * Tar remove command
  */
#ifndef REMOVE_H
#define REMOVE_H

/**
  * Set the global variable CMD_NAME_REMOVE
  * Copy STR to CMD_NAME_REMOVE
  * It is usefull to print the good printing errors
  * @param str can be RM or MV
  */
void set_remove_cmd_name(const char *str);

/**
  * Remove a file 
  * @param tar_name the tar in which we want to remove a file
  * @param filename the file we want to remove. If it is NULL, we want to remove the file tar_name.
  * @param options can be NULL or "r"
  * @return 0 on success; -1 otherwise
  */
int rm(char *tar_name, char *filename, char *options);

#endif
