/**
  * @file copy.h
  * Tar copy command
  */
#ifndef COPY_H
#define COPY_H


void set_cmd_name(char *str);

/**
 * Copy a file from a tar to a tar
 * 
 * @param src_tar path to the source tar
 * @param src_file path to the file in `src_tar`
 * @param dest_tar path to the dest tar
 * @param dest_file destination name (in `dest_tar`)
 * @param opt `NULL` for `cp` or "r" for `cp -r`
 */
int cp_tar_to_tar (char *src_tar, char *src_file, char *dest_tar, char *dest_file, char *opt);

/**
 * Copy an extern file to a tar
 * 
 * @param src_file path to the file to copy
 * @param dest_tar path to the dest tar
 * @param dest_file destination name (in `dest_tar`)
 * @param opt `NULL` for `cp` or "r" for `cp -r`
 */
int cp_ext_to_tar (char *src_file, char *dest_tar, char *dest_file, char *opt);

/**
 * Extract a file from a tar
 * 
 * @param src_tar path to the source tar
 * @param src_file path to the file in `src_tar`
 * @param dest_file destination name
 * @param opt `NULL` for `cp` or "r" for `cp -r`
 */
int cp_tar_to_ext (char *src_tar, char *src_file, char *dest_file, char *opt);

#endif
