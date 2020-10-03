#ifndef ERRORS_H
#define ERRORS_H

/* Manage error caused by a file and return a pointer of pointers of characters. */
void *error_p(const char *, int[], int);

/* Manage error caused by a file, and close all the files opened, and return an integer. */
int error_pt(const char *, int[], int);

#endif
