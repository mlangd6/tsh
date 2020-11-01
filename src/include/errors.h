#ifndef ERRORS_H
#define ERRORS_H

/* Manage error caused by a file and return a pointer of pointers of characters. */
void *error_p(int[], int);

/* Manage error caused by a file, and close all the files opened, and return an integer. */
int error_pt(int[], int);

/* Launch perror with name of command and problem */
void error_cmd(const char *cmd_name, const char *msg);
#endif
