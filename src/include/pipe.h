/**
 * @file pipe.h
 * Pipes handler.
 */

#ifndef PIPE_H
#define PIPE_H

#include "list.h"

/**
 * Execute all the pipes in the list of array of token.
 * The list should have at least 2 elements.
 * @param tokens the list of array of token.
 * @return 0 on success, -1 else.
 */
int exec_pipe(list *tokens);

#endif
