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

static void free_tokens_array(void *arr)
{
  array *val = arr;
  array_free(val, false);
}

int exec_line(char *line)
{
  list *tokens = tokenize(line);
  if (!parse_tokens(tokens))
  {
    free(line);
    list_free_full(tokens, free_tokens_array);
    return -1;
  }
  int nb_cmd = list_size(tokens);
  if (nb_cmd > 1)
  {
    return exec_pipe(tokens);
  }
  else
  {
    array *cmd_arr = list_remove_first(tokens);
    int argc = array_size(cmd_arr) - 1;
    list_free(tokens, false); // List empty
    exec_red_array(cmd_arr);
    remove_all_redir(cmd_arr);
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
        exec_red_array(cmd_arr);
        remove_all_redir(cmd_arr);
        exec_cmd_array(cmd_arr);
        token *first = array_get(cmd_arr, 0);
        char *cmd_name = first -> val.arg;
        free(first);
        if (errno == ENOENT)
        {
          char err[8192];
          sprintf(err, "%s: command not found\n", cmd_name);
          write(STDERR_FILENO, err, strlen(err) + 1);
        }
        else {
          perror(cmd_name);
        }
        exit(EXIT_FAILURE);
      }
      default: // Parent
      {
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
    return execv(cmd_exec, argv);
  }
  else
  {
    return execvp(argv[0], argv);
  }

  if (errno == ENOENT)
  {
    int size = strlen(argv[0]) + CMD_NOT_FOUND_SIZE;
    char error_msg[size];
    strcpy(error_msg, argv[0]);
    strcat(error_msg, CMD_NOT_FOUND);
    write(STDOUT_FILENO, error_msg, size);
  }

  exit(EXIT_FAILURE);
  return 0;
}

int exec_red_array(array *cmd)
{
  token *prev, *cur;
  bool prev_is_redir = false;
  int size = array_size(cmd);
  for (int i = 0; i < size - 1; i++) // Dernier élément est de type PIPE
  {
    cur = array_get(cmd, i);

    if (cur -> type == REDIR)
    {
      if (prev_is_redir)
      {
        write(STDERR_FILENO, "tsh: syntax error: unexpected token after redirection\n", 54);
        return -1;
      }
      prev = cur;
      prev_is_redir = true;
    } else {
      if (prev_is_redir)
      {
        if (launch_redir(prev -> val.red, cur -> val.arg) != 0)
        {
          return -1;
        }
      }
      prev_is_redir = false;
    }
  }
  if (prev_is_redir)
  {
    write(STDERR_FILENO, "tsh: syntax error: unexpected token after redirection\n", 54);
    return -1;
  }
  return 0;
}

void remove_all_redir(array *cmd)
{
  int size = array_size(cmd);
  token *tok;
  bool prev_is_redir = false;
  for (int i = 0; i < size; i++)
  {
    if ((tok = array_get(cmd, i)) -> type == REDIR)
    {
      prev_is_redir = true;
      tok = array_remove(cmd, i--);
      size--;
    }
    else if (prev_is_redir)
    {
      array_remove(cmd, i--);
      prev_is_redir = false;
      size--;
    }
    free(tok);
  }
}
