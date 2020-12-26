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

static bool well_formatted(void *a);
static token char_to_token(char *w);

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

static token char_to_token(char *w)
{
  token res;
  if (!strcmp(w, ">")) {
    res.val.red = STDOUT_REDIR;
    res.type = REDIR;
  } else if (!strcmp(w, "2>")) {
    res.val.red = STDERR_REDIR;
    res.type = REDIR;
  } else if (!strcmp(w, ">>")) {
    res.val.red = STDOUT_APPEND;
    res.type = REDIR;
  } else if (!strcmp(w, "2>>")) {
    res.val.red = STDERR_APPEND;
    res.type = REDIR;
  } else if (!strcmp(w, "<")) {
    res.val.red = STDIN_REDIR;
    res.type = REDIR;
  } else if (!strcmp(w, "|")) {
    res.type = PIPE;
  } else {
    res.val.arg = w;
    res.type = ARG;
  }
  return res;
}


list *tokenize(char *user_input)
{
  const char delim[] = " ";
  list *res = list_create();
  array *cur_arr = array_create(sizeof(token));
  token cur_tok = char_to_token(strtok(user_input, delim));
  array_insert_last(cur_arr, &cur_tok);
  list_insert_last(res, cur_arr);
  char *iter;
  while((iter = strtok(NULL, delim)))
  {
    if (cur_tok.type == PIPE)
    {
      cur_arr = array_create(sizeof(token));
      list_insert_last(res, cur_arr);
    }
    cur_tok = char_to_token(iter);
    array_insert_last(cur_arr, &cur_tok);
  }
  // Pourt garder une cohérence on ajoute un pipe à la fin, chaque cellule finit donc par un pipe
  if (cur_tok.type == PIPE)
  {
    cur_arr = array_create(sizeof(token));
    array_insert_last(cur_arr, &cur_tok);
    list_insert_last(res, cur_arr);
  }
  else {
    cur_tok.type = PIPE;
    array_insert_last(cur_arr, &cur_tok);
  }
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

bool parse_tokens(list *cmds)
{
  return list_for_all(cmds, well_formatted);
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
    free(it);
  }
  argv[size-1] = NULL;
  return argv;
}

static bool well_formatted(void *a)
{
  array *arr = a;
  int size = array_size(arr);
  if (size <= 1)
  {
    write(STDERR_FILENO, "tsh: syntax error: unexpected token near |\n", 44);
    return false;
  }
  bool prev_is_redir = false;
  token *cur;
  for (int i = 0; i < size; i++)
  {
    cur = array_get(arr, i);
    if (prev_is_redir)
    {
      if (cur -> type != ARG)
      {
        write(STDERR_FILENO, "tsh: syntax error: unexpected token after redirection\n", 54);
        return false;
      }
    }
    prev_is_redir = cur -> type == REDIR;
    free(cur);
  }
  return true;


}
