/**
  * @file copy.h
  * Tar copy command
  */
#ifndef COPY_H
#define COPY_H


#define SUPPORT_OPT "r"
char cmd_name[10];

/**
  * copy a file SRC_FILE from a tarball SRC_TAR to a tarball DEST_TAR as DEST_FILE
  * OPT could be NULL for "cp" or r for "cp -r"
  */
int cp_tar_to_tar (char *src_tar, char *src_file, char *dest_tar, char *dest_file, char *opt);

/**
  * copy a file SRC_FILE from the exterior of a tarball to the tarball DEST_TAR as DEST_FILE
  * OPT could be NULL for "cp" or r for "cp -r"
  */
int cp_ext_to_tar (char *src_file, char *dest_tar, char *dest_file, char *opt);

/**
  * copy a file SRC_FILE from a tarball SRC_TAR to the exterior of the tarball as DEST_FILE
  * OPT could be NULL for "cp" or r for "cp -r"
  */
int cp_tar_to_ext (char *src_tar, char *src_file, char *dest_file, char *opt);

#endif
