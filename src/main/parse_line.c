#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <ctype.h>

#include "tsh.h"
#include "tokens.h"
#include "list.h"
#include "array.h"

static token *char_to_token(char *w);
static void free_tokens(token **tokens, int start, int size);

int count_words(const char *str)
{
  int wc = 0;
  int in_word = 0;

  const char *s = str;


  while (*s)
  {
    if (isspace(*s))
    {
      in_word = 0;
    }
    else if (in_word == 0)
    {
      wc++;
      in_word = 1;
    }

    s++;
  }

  return wc;
}

static token *char_to_token(char *w)
{
  token *res = malloc(sizeof(token));
  if (!strcmp(w, ">")) {
    res -> val.red = STDOUT_REDIR;
    res -> type = REDIR;
  } else if (!strcmp(w, "2>")) {
    res -> val.red = STDERR_REDIR;
    res -> type = REDIR;
  } else if (!strcmp(w, ">>")) {
    res -> val.red = STDOUT_APPEND;
    res -> type = REDIR;
  } else if (!strcmp(w, "2>>")) {
    res -> val.red = STDERR_APPEND;
    res -> type = REDIR;
  } else if (!strcmp(w, "<")) {
    res -> val.red = STDIN_REDIR;
    res -> type = REDIR;
  } else if (!strcmp(w, "|")) {
    res -> type = PIPE;
  } else {
    res -> val.arg = w;
    res -> type = ARG;
  }
  return res;
}


list *tokenize(char *user_input)
{
  const char delim[] = " ";
  list *res = list_create();
  array *cur_arr = array_create(16);
  token *cur_tok = char_to_token(strtok(user_input, delim));
  array_insert_last(cur_arr, cur_tok);
  list_insert_last(res, cur_arr);
  char *iter;
  while((iter = strtok(NULL, delim)))
  {
    if (cur_tok -> type == PIPE)
    {
      cur_arr = array_create(16);
      list_insert_last(res, cur_arr);
    }
    cur_tok = char_to_token(iter);
    array_insert_last(cur_arr, cur_tok);
  }
  // Pourt garder une cohérence on ajoute un pipe à la fin, chaque cellule finit donc par un pipe
  cur_tok = malloc(sizeof(token));
  cur_tok -> type = PIPE;
  array_insert_last(cur_arr, cur_tok);
  return res;
}

char **array_to_argv(array *arr)
{
  int size = array_size(arr);
  char **argv = malloc(size * sizeof(char *));
  for (int i = 0; i < size-1; i++)
  {

  }
  argv[size-1] = NULL;
  return argv;
}

int exec_tokens(token **tokens, int nb_el, char **argv)
{
  short prev_is_redir = 0;
  int j = 0;
  for (int i = 0; i < nb_el; i++)
  {
    if (tokens[i] -> type == REDIR)
    {
      if (prev_is_redir)
      {
        write(STDERR_FILENO, "tsh: syntax error: unexpected token after redirection\n", 54);
        return -1;
      }
      prev_is_redir = 1;
    } else {
      if (prev_is_redir)
      {
        if (launch_redir(tokens[i-1] -> val.red, tokens[i] -> val.arg) != 0)
        {
          free_tokens(tokens, i-1, nb_el);
          return -1;
        }
        free(tokens[i-1]);
      }else {
        argv[j++] = tokens[i] -> val.arg;
      }
      free(tokens[i]);
      prev_is_redir = 0;
    }
  }
  if (prev_is_redir){
    write(STDERR_FILENO, "tsh: syntax error: unexpected token after redirection\n", 54);
    return -1;
  }
  argv[j] = NULL;
  return j;
}

char **cmd_array_to_argv(array *cmd_arr)
{
  int size = array_size(cmd_arr);
  char **argv = malloc(sizeof(char *) * size);
  token *it;
  for (int i = 0; i < size-1; i++)
  {
    it = array_get(cmd_arr, i);
    argv[i] = it -> val.arg;
  }
  argv[size-1] = NULL;
  return argv;
}

static void free_tokens(token **tokens, int start, int size)
{
  for (int i = start; i < size; i++)
  {
    free(tokens[i]);
  }
}
