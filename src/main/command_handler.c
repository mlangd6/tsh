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


enum arg_type {
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


static char *copy_string(const char *str);
static char *make_absolute(const char *path);
static int is_tar_path(char *path);

static struct arg *tokenize_args (int argc, char **argv);
static void free_tokens(struct arg *tokens, int tokens_size);

static void init_arg_info_options(arg_info *info, struct arg *tokens, int tokens_size);
static void init_arg_info(arg_info *info, struct arg *tokens, int tokens_size);

static char *get_opt(int optc, char **optv, char *optstring);

static int it_out(command *cmd, arg_info *info, char *arg);
static int it_tar(command *cmd, arg_info *info, char *arg);

static void invalid_options(char *cmd_name);


/* static int handle_with_pwd(command *cmd, char **argv, int not_tar_opt, char *tar_opt); */
/* static int handle_one_it_tar(command *cmd, char *arg, char *path, char *in_tar, */
/* 			     char *tar_opt, int not_tar_opt, int nb_valid_arg, int *rest); */
/* static int handle_one_it_out(command *cmd, int nb_valid_arg, char **argv, int argc, int *rest); */

/* static void print_arg_before(command *cmd, int nb_valid_arg, char *argv); */
/* static void print_arg_after(command *cmd, int *rest); */



static char *copy_string(const char *str)
{
  char *cpy = malloc(strlen(str)+1);
  assert(cpy);

  strcpy(cpy, str);
  return cpy;
}


static char *make_absolute(const char *path)
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


static int is_tar_path(char *path)
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
	      tokens[i].value = NULL;
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

static void free_tokens(struct arg *tokens, int tokens_size)
{
  for (int i=0; i < tokens_size; i++)
    {
      if (tokens[i].type != ERROR)
	free(tokens[i].value);
    }
  
  free(tokens);
}

static void init_arg_info_options(arg_info *info, struct arg *tokens, int tokens_size)
{
  info->options = malloc(info->opt_c * sizeof(char*));

  info->options[0] = tokens[0].value;
  
  for (int i=1, j=1; i < tokens_size; i++)
    {
      if (tokens[i].type == OPTION)
	info->options[j++] = tokens[i].value;
    }
}

static void init_arg_info(arg_info *info, struct arg *tokens, int tokens_size)
{
  info->nb_in_tar = 0;
  info->nb_out = 0;
  info->err_arg = false;
  info->opt_c = 0;
  info->options = NULL;

  for (int i=0; i < tokens_size; i++)
    {
      switch (tokens[i].type)
	{
	case CMD:
	case OPTION:
	  info->opt_c++;
	  break;

	case TAR_FILE:
	  info->nb_in_tar++;
	  break;

	case REG_FILE:
	  info->nb_out++;
	  break;

	case ERROR:
	  info->err_arg = true;
	  break;
	}
    }
  
  init_arg_info_options(info, tokens, tokens_size);
}

static int it_out(command *cmd, arg_info *info, char *arg)
{
  int wstatus, ret;
  pid_t cpid;

  // TODO : mettre dans une fonction
  char **exec_argv = malloc((info->opt_c + 2)*sizeof(char*));
  int i = 0;
  for (; i < info->opt_c; i++)
    {
      exec_argv[i] = info->options[i];
    }
  exec_argv[i++] = arg;
  exec_argv[i] = NULL;
  

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

static char *get_opt(int optc, char **optv, char *optstring)
{
  int c;
  char *detected_opt = malloc(strlen(optstring) + 1);
  assert(detected_opt);

  *detected_opt = '\0';
  int i = 0;
  while ((c = getopt(optc, optv, optstring)) != -1)
    {
      if (c == '?')
	{
	  free(detected_opt);
	  return NULL;
	}
      else if (strchr(detected_opt, c) == NULL)
	{
	  detected_opt[i++] = c;
	  detected_opt[i] = '\0';
	}
    }
  
  return detected_opt;
}

static int it_tar(command *cmd, arg_info *info, char *arg)
{
  char *detected_opt = get_opt(info->opt_c, info->options, cmd->support_opt);
  int ret;

  if (!detected_opt)
    {
      //invalid_options(cmd->name);
      return EXIT_FAILURE;
    }

  char *in_tar = split_tar_abs_path(arg);
  
  ret = cmd->in_tar_func(arg, in_tar, detected_opt);
  free (detected_opt);
  
  return ret;
}

int handle(command cmd, int argc, char **argv)
{
  int ret;
  arg_info info;
  struct arg *tokens = tokenize_args(argc, argv);

  init_arg_info(&info, tokens, argc);
  
  // Pas de tar en jeu
  if (info.nb_in_tar == 0 && (!cmd.twd_arg || info.nb_out > 0))
    {
      // Pas d'erreur
      if (info.nb_out > 0 || !info.err_arg)
	{
	  //TODO : mettre une fonction
	  for (int i=0; i < argc; i++)
	    argv[i] = tokens[i].value;

	  free(tokens);
	  
	  return execvp(cmd.name, argv);
	}
    
      return EXIT_FAILURE;
    }

  /* if (info.nb_in_tar == 0 && info.nb_out == 0) // No arguments (cmd.twd_arg == 1) */
  /* { */
  /*   if (info.err_arg) */
  /*     return EXIT_FAILURE; */
  /*   return handle_with_pwd(&cmd, argv, not_tar_opt, tar_opt); */
  /* } */

  
  for (int i = 0; i < argc; i++)
    {
      switch (tokens[i].type)
	{
	case CMD:
	case OPTION:
	  break;

	case ERROR:
	  error_cmd(cmd.name, argv[i]);
	  break;

	case TAR_FILE:
	  ret = it_tar(&cmd, &info, tokens[i].value);
	  break;
	  
	case REG_FILE:
	  ret = it_out(&cmd, &info, tokens[i].value);
	  break;
	}      
    }

  free_tokens(tokens, argc);
  free(info.options);
  
  return ret;
}

static void invalid_options(char *cmd_name)
{
  write(STDERR_FILENO, cmd_name, strlen(cmd_name));
  write(STDERR_FILENO, ": invalid option -- '", 22);
  write(STDERR_FILENO, &optopt, 1);
  write(STDERR_FILENO, "' with tarball, skipping files inside tarball\n", 46);
}

/* static int handle_with_pwd(command *cmd, char **argv, int not_tar_opt, char *tar_opt) */
/* { */
/*   char *tmp = getenv("PWD"); */
/*   char twd[PATH_MAX]; */
/*   memcpy(twd, tmp, strlen(tmp) + 1); */
/*   char *in_tar = split_tar_abs_path(twd); */
/*   if (in_tar != NULL) */
/*     { */
/*       if (not_tar_opt) */
/* 	{ */
/* 	  invalid_options(cmd -> name); */
/* 	  return EXIT_FAILURE; */
/* 	} */
/*       return cmd -> in_tar_func(twd, in_tar, tar_opt); */
/*     } */
/*   else execvp(cmd -> name, argv); */
/*   return EXIT_FAILURE; */
/* } */

/* static int handle_one_it_tar(command *cmd, char *arg, char *path, char *in_tar, */
/* 			     char *tar_opt, int not_tar_opt, int nb_valid_arg, int *rest) */
/* { */
/*   if (!not_tar_opt) { */
/*     print_arg_before(cmd, nb_valid_arg, arg); */
/*     int ret = cmd -> in_tar_func(path, in_tar, tar_opt); */
/*     print_arg_after(cmd, rest); */
/*     return ret; */
/*   } */
/*   return EXIT_FAILURE; */
/* } */

/* static void print_arg_after(command *cmd, int *rest) */
/* { */
/*   if (--(*rest) > 0 && cmd -> print_multiple_arg) */
/*     write(STDOUT_FILENO, "\n", 1); */
/* } */

/* static void print_arg_before(command *cmd, int nb_valid_arg, char *arg) */
/* { */
/*   if (cmd -> print_multiple_arg && nb_valid_arg > 1) */
/*     { */
/*       write(STDOUT_FILENO, arg, strlen(arg)); */
/*       write(STDOUT_FILENO, ": \n", 3); */
/*     } */
/* } */

/* static int handle_one_it_out(command *cmd, int nb_valid_arg, char **argv, int argc, int *rest) */
/* { */
/*   print_arg_before(cmd, nb_valid_arg, argv[argc]); */
/*   int status, p = fork(); */
/*   int ret = EXIT_SUCCESS; */
/*   switch(p) */
/*     { */
/*     case -1: */
/*       error_cmd(cmd -> name, "fork"); */
/*       break; */
/*     case 0: // child */
/*       execvp(cmd -> name, argv); */
/*       break; */
/*     default: // parent */
/*       wait(&status); */
/*       if (WEXITSTATUS(status) != EXIT_SUCCESS) */
/* 	ret = EXIT_FAILURE; */
/*     } */
/*   print_arg_after(cmd, rest); */
/*   return ret; */
/* } */
