/**
  * @file remove.h
  * Tar remove command
  */
#ifndef REMOVE_H
#define REMOVE_H

#define CMD_NAME_ "rm"
#define SUPPORT_OPT_ "r"

int rm(char *tar_name, char *filename, char *options);

#endif
