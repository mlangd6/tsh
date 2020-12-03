#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <ctype.h>

#include "tsh.h"
#include "parse_line.h"

static token *char_to_token(char *w); 

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
  } else {
    res -> val.arg = w;
    res -> type = ARG;
  }
  return res;
}


token **tokenize(char *user_input, int *nb_el)
{
  const char delim[] = " ";
  *nb_el = count_words(user_input);
  token **res = malloc((*nb_el+1) * sizeof(token *)); // (res[nb_tokens] = NULL)
  res[0] = char_to_token(strtok(user_input, delim));
  for (int i = 1; i < *nb_el; i++)
  {
    res[i] = char_to_token(strtok(NULL, delim));
  }
  res[*nb_el] = NULL;
  return res;
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
        launch_redir_before(tokens[i-1] -> val.red, tokens[i] -> val.arg);
        free(tokens[i-1]);
      }else {
        argv[j++] = tokens[i] -> val.arg;
      }
      free(tokens[i]);
      prev_is_redir = 0;
    }
  }
  if (prev_is_redir){
    write(STDERR_FILENO, "tsh: syntax error: unexpected token after redirection", 53);
    return -1;
  }
  argv[j] = NULL;
  return j;
}
