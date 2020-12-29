/**
  * @file remove.h
  * Tar remove command
  */
#ifndef REMOVE_H
#define REMOVE_H

#define SUPPORT_OPT_ "r"
char cmd_name_remove[3];

int rm(char *tar_name, char *filename, char *options);

#endif
