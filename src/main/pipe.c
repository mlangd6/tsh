#include <unistd.h>
#include <stdlib.h>

#include "pipe.h"
#include "redirection.h"
#include "tokens.h"
#include "errors.h"
#include "list.h"
#include "array.h"

static void close_pipes(int pipes_fd[][2], int size);

int exec_pipe(list *tokens)
{
  array *cmd_it;
  int size = list_size(tokens);
  int wstatus, cpid;
  int pipes_fd[size-1][2];
  for (int i = 0; i < size - 1; i++)
  {
    if (pipe(pipes_fd[i]) != 0)
    {
      error_cmd("tsh", "pipe");
      close_pipes(pipes_fd, i);
      return -1;
    }
  }
  for (int i = 0; i < size; i++)
  {
    cmd_it = list_remove_first(tokens);
    switch((cpid = fork()))
    {
      case -1:
        error_cmd("tsh", "fork");
        break;
      case 0: // Child
        if (i != 0)
        {
          add_reset_redir(STDIN_FILENO, 0);
          dup2(STDIN_FILENO, pipes_fd[i-1][1]);
        }
        if (i != size -1)
        {
          add_reset_redir(STDOUT_FILENO, 0);
          dup2(STDOUT_FILENO, pipes_fd[i][0]);
        }
        exec_cmd_array(cmd_it);
        break;
      default: // Parent

        break;

    }
  }
  return 0;
}

static void close_pipes(int pipes_fd[][2], int size)
{
  for (int i = 0; i < size; i++)
  {
    close(pipes_fd[i][0]);
    close(pipes_fd[i][1]);
  }
}
