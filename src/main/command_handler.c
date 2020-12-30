#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "command_handler.h"

#include "errors.h"
#include "path_lib.h"
#include "tar.h"
#include "utils.h"


static void init_arg_info_options (arg_info *info, struct arg *tokens, int tokens_size);


char *check_options (int argc, char **argv, char *optstring)
{  
  int c;
  char *detected_opt = malloc(strlen(optstring) + 1);
  assert(detected_opt);

  *detected_opt = '\0';
  int i = 0;

  /* On boucle sur les options.
     On continue même s'il y a des options invalides car getopt
     permute les options pour les placer au début. */
  while ((c = getopt(argc, argv, optstring)) != -1)
    {
      if (c == '?' && detected_opt)
	{	  
	  free (detected_opt);
	  detected_opt = NULL;
	}
      else if (detected_opt && strchr(detected_opt, c) == NULL)
	{
	  detected_opt[i++] = c;
	  detected_opt[i] = '\0';
	}
    }
  
  return detected_opt;
}

void invalid_options (char *cmd_name)
{
  write_string (STDERR_FILENO, cmd_name);
  write_string (STDERR_FILENO, ": invalid option -- '");
  
  write(STDERR_FILENO, &optopt, 1);

  write_string (STDERR_FILENO, "' with tarball, skipping files inside tarball\n");
}



struct arg *tokenize_args (int *argc, char **argv, arg_info *info)
{
  struct arg *tokens = malloc(*argc * sizeof(struct arg));
  assert(tokens);

  // Par définition, le premier argument est la commande
  tokens[0].value = copy_string(argv[0]);
  tokens[0].type = CMD;

  int j = 1;
  for (int i = 1; i < *argc; i++)
    {
      // OPTION
      if (*argv[i] == '-')
	{
	  tokens[j].value = copy_string (argv[i]);
	  tokens[j++].type = OPTION;
	}
      // ERROR ou TAR_FILE ou REG_FILE
      else
	{
	  char *abs, *reduce, *in_tar;

	  abs = make_absolute(argv[i]);
	  reduce = reduce_abs_path(abs, NULL);
	  in_tar = split_tar_abs_path (reduce);
	  
	  free(abs);

	  // ERROR
	  if (!reduce)
	    {
	      error_cmd(argv[0], argv[i]);
	      info->has_error = true;
	    }
	  // TAR_FILE
	  else if (in_tar) 
	    {	      
	      tokens[j].tf.tar_name = reduce;
	      tokens[j].tf.filename = in_tar;
	      tokens[j++].type = TAR_FILE;
	    }
	  // REG_FILE
	  else
	    {
	      tokens[j].value = reduce;
	      tokens[j++].type = REG_FILE;
	    }
	}
    }

  *argc = j;
  
  return tokens;
}

void free_tokens (struct arg *tokens, int argc)
{
  for (int i=0; i < argc; i++)
    {
      if (tokens[i].type == TAR_FILE)
	{
	  free( tokens[i].tf.tar_name);
	}
      else
	{
	  free (tokens[i].value);
	}
    }
  
  free(tokens);
}

int execvp_tokens (char *cmd_name, struct arg *tokens, int tokens_size)
{
  char *argv[tokens_size + 1];
  int i;

  argv[0] = cmd_name;
  
  for (i=1; i < tokens_size; i++)
    {
      if (tokens[i].type == TAR_FILE)
	{
	  if (tokens[i].tf.filename[0])
	    tokens[i].tf.filename[-1] = '/';

	  argv[i] = tokens[i].tf.tar_name;
	}
      else
	{
	  argv[i] = tokens[i].value;
	}
    }
  
  argv[i] = NULL;
  
  return execvp(argv[0], argv);
}

/** Init the field `char** options` of a `struct arg_info` from given tokens. */
static void init_arg_info_options (arg_info *info, struct arg *tokens, int tokens_size)
{
  info->options = malloc(info->options_size * sizeof(char*));

  info->options[0] = tokens[0].value;
  
  for (int i=1, j=1; i < tokens_size; i++)
    {
      if (tokens[i].type == OPTION)
	info->options[j++] = tokens[i].value;
    }
}

void init_arg_info (arg_info *info, struct arg *tokens, int tokens_size)
{
  info->nb_tar_file = 0;
  info->nb_reg_file = 0;
  info->options_size = 0;
  info->options = NULL;

  for (int i=0; i < tokens_size; i++)
    {
      switch (tokens[i].type)
	{
	case CMD:
	case OPTION:
	  info->options_size++;
	  break;

	case TAR_FILE:
	  info->nb_tar_file++;
	  break;

	case REG_FILE:
	  info->nb_reg_file++;
	  break;
	}
    }
  
  init_arg_info_options(info, tokens, tokens_size);
}

int get_nb_valid_file (arg_info *info, char *options)
{
  return info->nb_reg_file + (options ? info->nb_tar_file : 0);
}

bool no_arg (arg_info *info)
{
  return info->nb_tar_file == 0 && info->nb_reg_file == 0;
}
