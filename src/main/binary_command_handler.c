#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "command_handler.h"

#include "tar.h"
#include "errors.h"
#include "utils.h"
#include "path_lib.h"


static char** arg_info_to_argv (arg_info *info, char *arg, char *last);
  
static int handle_arg (binary_command *cmd, struct arg *token, struct arg *last_token, char *options, arg_info *info);
static int handle_reg_file (binary_command *cmd, arg_info *info, char *arg, char *last);
static int handle_tokens (binary_command *cmd, struct arg *tokens, int argc, arg_info *info, char *options);

static void free_all (struct arg *tokens, int argc, arg_info *info, char *options);

static int check_arg_existence (struct arg *token);

/** 
 * Gets a malloc'd array of string suitable for calling `execvp` from a `struct arg_info` and two strings.
 * 
 * The returned array looks like : `[info.options[0], ... ,info.options[info.options_size - 1], arg, last, NULL]`
 */
static char** arg_info_to_argv (arg_info *info, char *arg, char *last)
{
  char **exec_argv = malloc((info->options_size + 3)*sizeof(char*));
  assert (exec_argv);
  
  int i = 0;
  for (; i < info->options_size; i++)
    {
      exec_argv[i] = info->options[i];
    }
  
  exec_argv[i++] = arg;
  exec_argv[i++] = last;
  exec_argv[i] = NULL;

  return exec_argv;
}


static int handle_reg_file (binary_command *cmd, arg_info *info, char *arg, char *last)
{
  int wstatus, ret;
  pid_t cpid;
  char **exec_argv;

  exec_argv = arg_info_to_argv (info, arg, last);
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

static int handle_arg (binary_command *cmd, struct arg *token, struct arg *last_token, char *options, arg_info *info)
{
  int ret;
  
  if (token->type == REG_FILE && last_token->type == REG_FILE)
    {
      ret = handle_reg_file (cmd, info, token->value, last_token->value);
    }
  else if (token->type == REG_FILE && last_token->type == TAR_FILE)
    {      
      ret = cmd->extern_to_tar (token->value, last_token->tf.tar_name, last_token->tf.filename, options);     
    }
  else if (token->type == TAR_FILE && last_token->type == REG_FILE)
    {
      ret = cmd->tar_to_extern (token->tf.tar_name, token->tf.filename, last_token->value, options);
    }
  else // token->type == TAR_FILE && last_token->type == TAR_FILE
    {
      ret = cmd->tar_to_tar (token->tf.tar_name, token->tf.filename, last_token->tf.tar_name, last_token->tf.filename, options);
    }

  return ret;
}

static int handle_tokens (binary_command *cmd, struct arg *tokens, int argc, arg_info *info, char *options)
{
  int ret = EXIT_SUCCESS;
  struct arg *last_token = tokens + (argc - 1);
  
  if (!options)
    invalid_options (cmd->name);
  
  for (int i = optind; i < argc - 1; i++)
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
	  ret = handle_arg (cmd, tokens + i, last_token, options, info);
	  break;
	}      
    }  

  return ret;
}

static void free_all (struct arg *tokens, int argc, arg_info *info, char *options)
{
  if (options)
    free (options);

  if (tokens)
    free_tokens (tokens, argc);

  if (info && info->options)
    free (info->options);
}

/** Checks if the value of `token` is an existing directory */
static int check_arg_existence (struct arg *token)
{
  if (token->type == TAR_FILE)
    {
      if (is_empty_string (token->tf.filename))
	return 1;
      
      switch (type_of_file (token->tf.tar_name, token->tf.filename, true))
	{
	case NONE:
	  errno = ENOENT;
	  return 0;

	case REG:
	  errno = ENOTDIR;
	  return 0;

	default:
	  return 1;
	}	
    }
  else
    {
      struct stat st;
      if (stat (token->value, &st) < 0)
	return 0;
      
      if (S_ISDIR (st.st_mode))
	return 1;

      errno = ENOTDIR;
      return 0;
    }
}

static void print_error (char *cmd_name, struct arg *token)
{
  switch (token->type)
    {
    case TAR_FILE:
      tar_error_cmd (cmd_name, token->tf.tar_name, token->tf.filename);
      break;

    case REG_FILE:
      error_cmd (cmd_name, token->value);
      break;

    default:
      break;
    }
}

int handle_binary_command (binary_command cmd, int argc, char **argv)
{
  int ret;
  char *tar_options;
  struct arg *tokens;
  struct arg *last_token;

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
  
  last_token = tokens + (argc - 1);

  init_arg_info (&info, tokens, argc);

  
  // Pas de tar en jeu
  if (info.nb_tar_file == 0)
    return execvp_tokens (cmd.name, tokens, argc);
    

  int nb_valid_file = get_nb_valid_file (&info, tar_options);
  if (nb_valid_file <= 1)
    {
      if (tar_options)
	write_string (STDERR_FILENO, "At least 2 valid arguments are needed !\n");
      else	
	write_string (STDERR_FILENO, "Invalid option and can't recover from error !\n");
            
      ret = EXIT_FAILURE;
    }
  else if (nb_valid_file > 2 && !check_arg_existence (last_token))
    {
      print_error (cmd.name, last_token);
      ret = EXIT_FAILURE;
    }
  else
    {
      ret = handle_tokens (&cmd, tokens, argc, &info, tar_options);
    }

  free_all (tokens, argc, &info, tar_options);
  
  return ret;
}
