#include <assert.h>
#include <getopt.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "command_handler.h"

#include "path_lib.h"
#include "errors.h"
#include "utils.h"

static char **arg_info_to_argv (arg_info *info, char *arg);

static void print_arg_before (unary_command *cmd, struct arg *token, int nb_valid_file);
static void print_arg_after (unary_command *cmd, int *rest);

static int handle_tokens (unary_command *cmd, struct arg *tokens, int argc, arg_info *info, char *options);
static int handle_arg (unary_command *cmd, struct arg *token, arg_info *info, char *options);
static int handle_reg_file (unary_command *cmd, arg_info *info, char *arg);
static int handle_tar_file (unary_command *cmd, char *tar_name, char *filename, char *detected_options);
static int handle_with_pwd (unary_command *cmd, char **argv, char *detected_options);

static void free_all (struct arg *tokens, int argc, arg_info *info, char *options);

/** 
 * Gets a malloc'd array of string suitable for calling `execvp` from a `struct arg_info` and a string.
 * 
 * The returned array looks like : `[info.options[0], ... ,info.options[info.options_size - 1], arg, NULL]`
 */
static char** arg_info_to_argv (arg_info *info, char *arg)
{
  char **exec_argv = malloc((info->options_size + 2)*sizeof(char*));
  assert (exec_argv);
  
  int i = 0;
  for (; i < info->options_size; i++)
    {
      exec_argv[i] = info->options[i];
    }
  
  exec_argv[i++] = arg;
  exec_argv[i] = NULL;

  return exec_argv;
}

/** Prints a string followed by a newline on `STDOUT_FILENO` */
static void print_arg_before (unary_command *cmd, struct arg *token, int nb_valid_file)
{
  if (cmd->print_multiple_arg && nb_valid_file > 1)
    {
      if (token->type == TAR_FILE)
	{
	  write_string (STDOUT_FILENO, token->tf.tar_name);
	  write_string (STDOUT_FILENO, "/");
	  write_string (STDOUT_FILENO, token->tf.filename);
	}
      else
	{
	  write_string (STDOUT_FILENO, token->value);
	}
      
      write_string (STDOUT_FILENO, ": \n");
    }
}

/** Prints `\n` on `STDOUT_FILENO` */
static void print_arg_after (unary_command *cmd, int *rest)
{
  if (--(*rest) > 0 && cmd->print_multiple_arg)
    write_string(STDOUT_FILENO, "\n");
}

/** Main routine : handles all tokens */
static int handle_tokens (unary_command *cmd, struct arg *tokens, int argc, arg_info *info, char *options)
{
  int ret;
  int nb_valid_file;
  int rest;

  
  ret = EXIT_SUCCESS;
  nb_valid_file = get_nb_valid_file (info, options);
  rest = nb_valid_file;
  
  if (!options)
    invalid_options (cmd->name);

  for (int i = optind; i < argc; i++)
    {      
      switch (tokens[i].type)
	{
	case CMD:
	case OPTION:
	  break;

	case TAR_FILE:
	  if (!options)
	    break;
	  // Attention on peut encore continuer
	case REG_FILE:
	  print_arg_before (cmd, tokens + i, nb_valid_file);
	  
	  ret = handle_arg (cmd, tokens + i, info, options);

	  print_arg_after (cmd, &rest);
	  break;
	}      
    }

  return ret;
}

static int handle_arg (unary_command *cmd, struct arg *token, arg_info *info, char *options)
{
  int ret;
  
  if (token->type == TAR_FILE)
    {
      ret = handle_tar_file (cmd, token->tf.tar_name, token->tf.filename, options);
    }
  else // token->type == REG_FILE
    {      
      ret = handle_reg_file (cmd, info, token->value);
    }

  return ret;
}

static int handle_reg_file (unary_command *cmd, arg_info *info, char *arg)
{
  int wstatus, ret;
  pid_t cpid;
  char **exec_argv;

  exec_argv = arg_info_to_argv (info, arg);
  ret = EXIT_SUCCESS;
  cpid = fork();
    
  switch(cpid)
    {
    case -1:
      error_cmd(cmd->name, "fork");
      break;
      
    case 0: // child
      execvp(cmd -> name, exec_argv);
      break;
      
    default: // parent
      wait(&wstatus);
      if (WEXITSTATUS(wstatus) != EXIT_SUCCESS)
	ret = EXIT_FAILURE;
    }

  free(exec_argv);
  return ret;
}

static int handle_tar_file (unary_command *cmd, char *tar_name, char *filename, char *detected_options)
{
  if (!detected_options)
    return EXIT_FAILURE;

  return cmd->in_tar_func (tar_name, filename, detected_options);
}

/** Handles a unary command with `PWD` */
static int handle_with_pwd (unary_command *cmd, char **argv, char *detected_options)
{
  int ret;
  char *pwd, *in_tar;

  pwd = copy_string(getenv("PWD"));  
  in_tar = split_tar_abs_path(pwd);
  
  if (in_tar)
    {
      if (detected_options)
	{
	  ret = cmd->in_tar_func(pwd, in_tar, detected_options);
	}
      else
	{
	  invalid_options (cmd->name);
	  ret = EXIT_FAILURE;
	}
      
      free(pwd);
    }
  else
    {
      free(pwd);
      execvp(cmd->name, argv);
    }

  return ret;
}

/** Free all passed arguments if not `NULL` */
static void free_all (struct arg *tokens, int argc, arg_info *info, char *options)
{
  if (options)
    free (options);

  if (tokens)
    free_tokens (tokens, argc);

  if (info && info->options)
    free (info->options);
}


int handle_unary_command (unary_command cmd, int argc, char **argv)
{
  int ret;
  char *tar_options;
  struct arg *tokens;
  
  arg_info info =
    {
      0,
      0,
      false,
      0,
      NULL
    };
  
  opterr = 0; // Pour ne pas avoir les messages d'erreurs de getopt

  ret = EXIT_SUCCESS;
  
  tar_options = check_options (argc, argv, cmd.support_opt); // on récupère les options pour la commande tar

  tokens = tokenize_args(&argc, argv, &info);

  init_arg_info(&info, tokens, argc);  

  if (no_arg(&info) && info.has_error)
    {
      free_all (tokens, argc, &info, tar_options);
      return EXIT_FAILURE;
    }
  
  // Pas d'argument et PWD
  if (no_arg(&info) && cmd.twd_arg)
    {
      ret = handle_with_pwd (&cmd, argv, tar_options);
      
      free_all (tokens, argc, &info, tar_options);

      return ret;
    }
    
  // Pas de tar en jeu
  if (info.nb_tar_file == 0)
    return execvp_tokens (cmd.name, tokens, argc);

  // Les autres cas
  ret = handle_tokens (&cmd, tokens, argc, &info, tar_options);
  
  free_all (tokens, argc, &info, tar_options);
  
  return ret;
}
