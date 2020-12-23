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

static int exec_red_array(array *cmd);
static void remove_all_redir(array *cmd);

int exec_line(char *line)
{
  list *tokens = tokenize(line);
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
    int is_special = special_command(first_tok -> val.arg);
    if (is_special == TSH_FUNC)
    {
      char **argv = cmd_array_to_argv(cmd_arr);
      array_free(cmd_arr, true);
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
        printf("CHILD\n");
        exec_cmd_array(cmd_arr);
      }
      default: // Parent
      {
        waitpid(cpid, &wstatus, 0);
        return WEXITSTATUS(wstatus);
      }

    }
  }
}

// int exec_line(char *line)
// {
  // int wstatus, is_special;
  // list *tokens;
  // char **args;
  // int nb_tokens;
  // int ret_value;
  // pid_t cpid;
//
  // if (!count_words(line))
    // return 0;
//
  // tokens = tokenize(line);
  // args = malloc((nb_tokens + 1) * sizeof(char *));
  // nb_tokens = exec_tokens(tokens, nb_tokens, args);
  // free(tokens);
  // if (nb_tokens <= 0) // No tokens or an error occured
  // {
    // free(line);
    // free(args);
    // reset_redirs(); // In case some redirections worked
    // return 0;
  // }
  // is_special = special_command(args[0]);
  // if (is_special == TSH_FUNC)
  // {
    // ret_value = launch_tsh_func(args, nb_tokens);
  // }
  // else
  // {
    // cpid = fork();
    // switch (cpid)
    // {
      // case -1:
      // perror ("tsh: fork");
      // exit (EXIT_FAILURE);
//
      // case 0: // child
      // if (is_special == TAR_CMD)
      // {
        // char cmd_exec[PATH_MAX];
        // char tsh_dir[PATH_MAX];
        // sprintf(cmd_exec, "%s/bin/%s", get_tsh_dir(tsh_dir), args[0]);
        // ret_value = execv(cmd_exec, args);
      // }
      // else
      // {
        // ret_value = execvp(args[0], args);
      // }
//
      // if (errno == ENOENT)
      // {
        // int size = strlen(args[0]) + CMD_NOT_FOUND_SIZE;
        // char error_msg[size];
        // strcpy(error_msg, args[0]);
        // strcat(error_msg, CMD_NOT_FOUND);
        // write(STDOUT_FILENO, error_msg, size);
      // }
//
      // exit(EXIT_FAILURE);
//
      // default: // parent
      // waitpid(cpid, &wstatus, 0);
      // ret_value = WEXITSTATUS(wstatus);
    // }
//
  // }
  // free(args);
  // free(line);
  // reset_redirs();
  // return ret_value;
// }

int exec_cmd_array(array *cmd)
{
  char **argv = cmd_array_to_argv(cmd);
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

static int exec_red_array(array *cmd)
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

static void remove_all_redir(array *cmd)
{
  int size = array_size(cmd);
  token *tok;
  for (int i = 0; i < size; i++)
  {
    if ((tok = array_get(cmd, i)) -> type == REDIR)
    {
      tok = array_remove(cmd, i--);
      free(tok);
      size--;
    }
  }
}
