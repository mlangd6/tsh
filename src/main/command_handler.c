#include <assert.h>
#include <getopt.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "command_handler.h"
#include "errors.h"
#include "path_lib.h"
#include "tar.h"


enum arg_type{
  CMD,
  TAR_FILE,
  REG_FILE,
  OPTION,
  ERROR
};


struct arg {
  char *value;
  enum arg_type type;
};

static int write_string (int fd, const char *string);
static char *copy_string (const char *str);

static char *make_absolute (const char *path);
static int is_tar_path (char *path);

static struct arg *tokenize_args (int argc, char **argv);
static void free_tokens (struct arg *tokens, int tokens_size);
static char **tokens_to_argv (struct arg *tokens, int tokens_size);

static void init_arg_info_options (arg_info *info, struct arg *tokens, int tokens_size);
static void init_arg_info (arg_info *info, struct arg *tokens, int tokens_size);
static char** arg_info_to_argv (arg_info *info, char *arg);
static int get_nb_valid_file (arg_info *info, char *options);
static bool no_arg (arg_info *info);

static char *check_options (int argc, char **argv, char *optstring);
static void invalid_options (char *cmd_name);

static int handle_arg (unary_command *cmd, struct arg *token, arg_info *info, char *options);
static int handle_reg_file (unary_command *cmd, arg_info *info, char *arg);
static int handle_tar_file (unary_command *cmd, char *arg, char *detected_options);
static int handle_with_pwd (unary_command *cmd, int argc, char **argv, char *detected_options);

static void print_arg_before (unary_command *cmd, char *arg, int nb_valid_file);
static void print_arg_after (unary_command *cmd, int *rest);

static void free_all (struct arg *tokens, int argc, arg_info *info, char *options);



/** Writes a string to a file descriptor. */
static int write_string (int fd, const char *string)
{
  return write(fd, string, strlen(string) + 1);
}

/** Gets a malloc'd copy of a string. */
static char *copy_string (const char *str)
{
  char *cpy = malloc(strlen(str)+1);
  assert(cpy);

  strcpy(cpy, str);
  return cpy;
}

/** Gets the absolute version of a path. */
static char *make_absolute (const char *path)
{
  char *abs;
  size_t path_len = strlen(path);
  
  if (*path == '/')
    {
      abs = copy_string(path);
    }
  else
    {
      char *pwd = getenv("PWD");
      abs = malloc(path_len + 2 + strlen(pwd));
      assert(abs);
      sprintf(abs, "%s/%s", pwd, path);
    }

  return abs;
}

/** Checks if an absolute path goes through a tar. */
static int is_tar_path (char *path)
{
  if (path == NULL || *path != '/')
    return -1;


  char *chr = path+1;
  bool tar_path = false;
  
  while (*chr && !tar_path)
    {
      if(*chr == '/')
	{
	  *chr = '\0';
      
	  if (is_tar(path) == 1)
	    tar_path = true;

	  *chr = '/';
	}
      chr++;
    }

  if (is_tar(path) == 1)
    tar_path = true;

  return tar_path;
}


static struct arg *tokenize_args (int argc, char **argv)
{
  struct arg *tokens = malloc(argc * sizeof(struct arg));
  assert(tokens);
  
  tokens[0].value = copy_string(argv[0]);
  tokens[0].type = CMD;

  for (int i = 1; i < argc; i++)
    {
      if (*argv[i] == '-')
	{
	  tokens[i].value = copy_string(argv[i]);
	  tokens[i].type = OPTION;
	}
      else
	{
	  char *abs, *reduce;

	  abs = make_absolute(argv[i]);
	  reduce = reduce_abs_path(abs, NULL);

	  free(abs);
	  
	  if (!reduce)
	    {
	      tokens[i].value = copy_string(argv[i]);
	      tokens[i].type = ERROR;	      
	    }
	  else
	    {
	      tokens[i].value = reduce;
	      tokens[i].type = is_tar_path(reduce) ? TAR_FILE : REG_FILE;
	    }
	}
    }

  return tokens;
}

static void free_tokens (struct arg *tokens, int argc)
{
  for (int i=0; i < argc; i++)
    {
      free(tokens[i].value);
    }
  
  free(tokens);
}

static char **tokens_to_argv (struct arg *tokens, int tokens_size)
{
  char **argv = malloc((tokens_size + 1)*sizeof(char*));
  assert(argv);
  int i;
  
  for (i=0; i < tokens_size; i++)
    {
      argv[i] = tokens[i].value;
    }
  argv[i] = NULL;

  return argv;
}


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

static void init_arg_info (arg_info *info, struct arg *tokens, int tokens_size)
{
  info->nb_tar_file = 0;
  info->nb_reg_file = 0;
  info->nb_error = 0;
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

	case ERROR:
	  info->nb_error++;
	  break;
	}
    }
  
  init_arg_info_options(info, tokens, tokens_size);
}

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

static int get_nb_valid_file (arg_info *info, char *options)
{
  return info->nb_reg_file + (options ? info->nb_tar_file : 0);
}

static bool no_arg (arg_info *info)
{
  return info->nb_tar_file == 0 && info->nb_reg_file == 0;
}


static char *check_options (int argc, char **argv, char *optstring)
{  
  int c;
  char *detected_opt = malloc(strlen(optstring) + 1);
  assert(detected_opt);

  *detected_opt = '\0';
  int i = 0;
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

static void invalid_options (char *cmd_name)
{
  write_string (STDERR_FILENO, cmd_name);
  write_string (STDERR_FILENO, ": invalid option -- '");
  
  write(STDERR_FILENO, &optopt, 1);

  write_string (STDERR_FILENO, "' with tarball, skipping files inside tarball\n");
}


static int handle_arg (unary_command *cmd, struct arg *token, arg_info *info, char *options)
{
  int ret;
  
  if (token->type == TAR_FILE)
    {
      ret = handle_tar_file (cmd, token->value, options);
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

static int handle_tar_file (unary_command *cmd, char *arg, char *detected_options)
{
  if (!detected_options)
    return EXIT_FAILURE;
  
  int ret;  
  char *in_tar = split_tar_abs_path(arg);
  
  if (!in_tar)
    return EXIT_FAILURE;
  
  ret = cmd->in_tar_func(arg, in_tar, detected_options);
  
  return ret;
}

static int handle_with_pwd (unary_command *cmd, int argc, char **argv, char *detected_options)
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


static void print_arg_before (unary_command *cmd, char *arg, int nb_valid_file)
{
  if (cmd->print_multiple_arg && nb_valid_file > 1)
    {  
      write_string (STDOUT_FILENO, arg);
      write_string (STDOUT_FILENO, ": \n");
    }
}

static void print_arg_after (unary_command *cmd, int *rest)
{
  if (--(*rest) > 0 && cmd->print_multiple_arg)
    write_string(STDOUT_FILENO, "\n");
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


int handle_unary_command (unary_command cmd, int argc, char **argv)
{
  int ret;
  char *tar_options;
  struct arg *tokens;
  arg_info info;

  
  opterr = 0; // Pour ne pas avoir les messages d'erreurs de getopt

  ret = EXIT_SUCCESS;
  
  tar_options = check_options (argc, argv, cmd.support_opt); // on récupère les options pour la commande tar

  tokens = tokenize_args(argc, argv);

  init_arg_info(&info, tokens, argc);

  
  // Pas de tar en jeu
  if (info.nb_tar_file == 0 && (!cmd.twd_arg || info.nb_reg_file > 0))
    {
      // Pas d'erreur
      if (info.nb_reg_file > 0 || info.nb_error == 0)
	{
	  char **exec_argv = tokens_to_argv(tokens, argc);
	  
	  free (tokens);
	  free_all (NULL, -1, &info, tar_options);

	  return execvp(cmd.name, exec_argv);
	}
    
      return EXIT_FAILURE;
    }
    
  // Pas d'arguments et PWD
  if (no_arg(&info) && cmd.twd_arg)
    {
      ret = handle_with_pwd (&cmd, argc, argv, tar_options);
      
      free_all (tokens, argc, &info, tar_options);

      return ret;
    }

  if (!tar_options)
    invalid_options (cmd.name);
  
  int nb_valid_file = get_nb_valid_file (&info, tar_options);
  int rest = nb_valid_file;
  for (int i = optind; i < argc; i++)
    {      
      switch (tokens[i].type)
	{
	case CMD:
	case OPTION:
	  break;

	case ERROR:
	  error_cmd (cmd.name, tokens[i].value);
	  break;

	case TAR_FILE:
	  if (!tar_options)
	    break;
	  // Attention on peut encore continuer
	case REG_FILE:
	  print_arg_before (&cmd, tokens[i].value, nb_valid_file);
	  
	  handle_arg (&cmd, tokens + i, &info, tar_options);

	  print_arg_after (&cmd, &rest);
	  break;
	}      
    }
  
  free_all (tokens, argc, &info, tar_options);
  
  return ret;
}

static int handle_binary_arg (binary_command *cmd, struct arg *token, struct arg *last_token, char *options, arg_info *info)
{
  int ret;
  
  if (token->type == REG_FILE && last_token->type == REG_FILE)
    {
      ret = EXIT_SUCCESS;
    }
  else if (token->type == REG_FILE && last_token->type == TAR_FILE)
    {
      char *dest_tar = copy_string (last_token->value);
      char *in_tar = split_tar_abs_path (dest_tar);
      ret = cmd->extern_to_tar (token->value, dest_tar, in_tar, options);
      free (dest_tar);
    }
  else if (token->type == TAR_FILE && last_token->type == REG_FILE)
    {
      char *src_tar = copy_string (token->value);
      char *in_tar = split_tar_abs_path (src_tar);
      ret = cmd->tar_to_extern (src_tar, in_tar, last_token->value, options);
      free (src_tar);
    }
  else // token->type == TAR_FILE && last_token->type == TAR_FILE
    {
      char *dest_tar = copy_string (last_token->value);
      char *dest_file = split_tar_abs_path (dest_tar);
      char *src_tar = copy_string (token->value);
      char *src_file = split_tar_abs_path (src_tar);
      ret = cmd->tar_to_tar (src_tar, src_file, dest_tar, dest_file, options);
      free (dest_tar);
      free (src_tar);
    }

  return ret;
}

int handle_binary_command (binary_command cmd, int argc, char **argv)
{
  int ret;
  char *tar_options;
  struct arg *tokens;
  struct arg *last_token;
  arg_info info;

  
  opterr = 0; // Pour ne pas avoir les messages d'erreurs de getopt

  ret = EXIT_SUCCESS;

  tar_options = check_options (argc, argv, cmd.support_opt); // on récupère les options pour la commande tar
  
  tokens = tokenize_args(argc, argv);
  
  last_token = tokens + (argc - 1);

  init_arg_info (&info, tokens, argc);

  
  // Pas de tar en jeu
  if (info.nb_tar_file == 0)
    {
      // TODO : mettre dans une fonction
      char **exec_argv = tokens_to_argv (tokens, argc);
      
      free (tokens);
      free_all (NULL, -1, &info, tar_options);

      return execvp (cmd.name, exec_argv);
    }
    

  int total_files = argc - info.options_size;

  if (total_files <= 1)
    {
      free_all (tokens, argc, &info, tar_options);
    }
  else if (total_files == 2)
    {
      if (!tar_options)
	{
	  free_all (tokens, argc, &info, tar_options);
	  write_string (STDERR_FILENO, "Invalid option and can't recover from error.\n");
	  return EXIT_FAILURE;	  
	}
      
      return handle_binary_arg (&cmd, tokens + optind, last_token, tar_options, &info);
    }

  // TODO vérifier que le dernier fichier existe
  
  if (!tar_options)
    invalid_options (cmd.name);
  
  for (int i = optind; i < argc; i++)
    {      
      switch (tokens[i].type)
	{
	case CMD:
	case OPTION:
	  break;

	case ERROR:
	  error_cmd (cmd.name, tokens[i].value);
	  break;

	case TAR_FILE:
	  if (!tar_options)
	    break;
	  // Attention on peut encore continuer
	case REG_FILE:	  
	  ret = handle_binary_arg (&cmd, tokens + i, last_token, tar_options, &info);
	  break;
	}      
    }

  free_all (tokens, argc, &info, tar_options);
  
  return ret;
}
