#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/limits.h>
#include <stdbool.h>


#include "tokens.h"
#include "tsh.h"
#include "list.h"
#include "array.h"
#include "pipe.h"
#include "errors.h"



int exec_line(char *line)
{
  list *tokens = tokenize(line);
  if (!parse_tokens(tokens))
  {
    free_tokens_list(tokens);
    free(line);
    return -1;
  }
  int nb_cmd = list_size(tokens);
  if (nb_cmd > 1)
  {
    int ret = exec_pipe(tokens);
    free(line);
    return ret;
  }
  else
  {
    array *cmd_arr = list_remove_first(tokens);
    int argc = array_size(cmd_arr) - 1;
    list_free(tokens, false); // List empty
    if (exec_red_array(cmd_arr) != 0)
    {
      array_free(cmd_arr, false);
      reset_redirs();
      return -1;
    }
    remove_all_redir_tokens(cmd_arr);
    if (array_size(cmd_arr) <= 1)
    {
      array_free(cmd_arr, false);
      reset_redirs();
      return EXIT_SUCCESS;
    }
    token *first_tok = array_get(cmd_arr, 0);
    char *cmd_name = first_tok -> val.arg;
    int is_special = special_command(cmd_name);
    free(first_tok);
    if (is_special == TSH_FUNC)
    {
      char **argv = cmd_array_to_argv(cmd_arr);
      array_free(cmd_arr, false);
      int ret = launch_tsh_func(argv, argc);
      free(argv);
      free(line);
      return ret;
    }
    int cpid, wstatus;
    switch ((cpid = fork()))
    {
      case -1:
      {
        error_cmd("tsh", "fork");
        return -1;
      }
      case 0: // Child
      {
        if (exec_red_array(cmd_arr) != 0)
        {
          array_free(cmd_arr, false);
          reset_redirs();
          return EXIT_FAILURE;
        }
        remove_all_redir_tokens(cmd_arr);
        exec_cmd_array(cmd_arr);
      }
      default: // Parent
      {
        free(line);
        array_free(cmd_arr, false);
        waitpid(cpid, &wstatus, 0);
        reset_redirs();
        return WEXITSTATUS(wstatus);
      }

    }
  }
}

int exec_cmd_array(array *cmd)
{
  char **argv = cmd_array_to_argv(cmd);
  array_free(cmd, false);
  int is_special = special_command(argv[0]);
  if (is_special == TAR_CMD)
  {
    char cmd_exec[PATH_MAX];
    char tsh_dir[PATH_MAX];
    sprintf(cmd_exec, "%s/bin/%s", get_tsh_dir(tsh_dir), argv[0]);
    execv(cmd_exec, argv);
  }
  else
  {
    execvp(argv[0], argv);
  }

  if (errno == ENOENT)
  {
    int size = strlen(argv[0]) + CMD_NOT_FOUND_SIZE;
    char error_msg[size];
    strcpy(error_msg, argv[0]);
    strcat(error_msg, CMD_NOT_FOUND);
    write(STDERR_FILENO, error_msg, size);
  }

  exit(EXIT_FAILURE);
  return 0;
}

int exec_red_array(array *cmd)
{
  token *prev = NULL, *cur;
  bool prev_is_redir = false;
  int size = array_size(cmd);
  for (int i = 0; i < size - 1; i++) // Dernier élément est de type PIPE
  {
    cur = array_get(cmd, i);

    if (cur -> type == REDIR)
    {
      if (prev_is_redir)
      {
        return -1;
      }
      prev_is_redir = true;
    } else {
      if (prev_is_redir)
      {
        if (launch_redir(prev -> val.red, cur -> val.arg) != 0)
        {
          free(cur);
          free(prev);
          return -1;
        }
      }
      prev_is_redir = false;
    }
    if (prev != NULL) free(prev);
    prev = cur;
  }
  free(cur);
  return 0;
}

void remove_all_redir_tokens(array *cmd)
{
  int size = array_size(cmd);
  token *tok;
  bool prev_is_redir = false;
  for (int i = 0; i < size; i++)
  {
    if ((tok = array_get(cmd, i)) -> type == REDIR)
    {
      prev_is_redir = true;
      free(tok);
      tok = array_remove(cmd, i--);
      size--;
    }
    else if (prev_is_redir)
    {
      free(array_remove(cmd, i--));
      prev_is_redir = false;
      size--;
    }
    free(tok);
  }
}
