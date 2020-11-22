#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <ctype.h>

#include "tsh.h"
#include "parse_line.h"

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


char **split(char *user_input, int *is_special)
{
  const char delim[] = " ";
  int nb_tokens = count_words(user_input);
  char **res = malloc((nb_tokens+1) * sizeof(char *)); // +1 pour le NULL à la fin
  char *tok, *src;

  res[0] = strtok(user_input, delim);

  *is_special = special_command(res[0]);


  for (int i = 1; i < nb_tokens; i++)
  {
    tok = strtok(NULL, delim);

    if (*is_special)
    {
      if (*tok != '/' && *tok != '-') // Relative path
      {
        src = malloc (PATH_MAX);
        strcpy (src, getenv("PWD"));
        strcat (src, "/");
        strcat (src, tok);
      }
      else //Absolute path or option
      {
        src = malloc (strlen(tok)+1);
        strcpy (src, tok);
      }
    }
    else
    {
      src = tok;
    }

    res[i] = src;
  }

  res[nb_tokens] = NULL;

  return res;
}
