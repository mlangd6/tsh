/**
  * @file remove.h
  * Tar remove command
  */
#ifndef REMOVE_H
#define REMOVE_H

#define SUPPORT_OPT_ "r"

void set_remove_cmd_name(const char *str);

int rm(char *tar_name, char *filename, char *options);

#endif
