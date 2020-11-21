#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <linux/limits.h>
#include "path_lib.h"
#include "tar.h"
#include "errors.h"

#define PROMPT "$ "
#define CMD_NOT_FOUND " : command not found\n"
#define CMD_NOT_FOUND_SIZE 22
#define NB_TAR_CMD 2

#define NB_TSH_FUNC 3
#define TAR_CMD 1
#define TSH_FUNC 2
#define MAX_TSH_FUNC_SIZE 5
#define SET_TSH_FUNC 1
#define NO_SET_TSH_FUNC 0

static int ret_value;

static int cd(char **argv, int argc);
static int exit_tsh(char **argv, int argc);
static int pwd(char **argv, int argc);

static int count_words(const char *str);
static char **split(char *user_input, int *is_special);
static void init_tsh();

static int special_command(char *s, char **tab, int tab_l, int set_tsh_func);
static int launch_tsh_func(char **argv, int argc);
static int count_argc(char **argv);


char tsh_dir[PATH_MAX];
char *tar_cmds[NB_TAR_CMD] = {"cat", "ls"};
char *tsh_funcs[NB_TSH_FUNC] = {"cd", "exit", "pwd"};
char twd[PATH_MAX];
char tsh_func[MAX_TSH_FUNC_SIZE];

static int count_words(const char *str)
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


static char **split(char *user_input, int *is_special)
{
  const char delim[] = " ";
  int nb_tokens = count_words(user_input);
  char **res = malloc((nb_tokens+1) * sizeof(char *)); // +1 pour le NULL à la fin
  char *tok, *src;

  res[0] = strtok(user_input, delim);

  *is_special = (special_command(res[0], tar_cmds, NB_TAR_CMD, NO_SET_TSH_FUNC)) ? 1 :
    (special_command(res[0], tsh_funcs, NB_TSH_FUNC, SET_TSH_FUNC)) ? 2 : 0;


  for (int i = 1; i < nb_tokens; i++)
    {
      tok = strtok(NULL, delim);

      if (*is_special)
	{
	  if (*tok != '/' && *tok != '-') // Relative path
	    {
	      src = malloc (PATH_MAX);
	      strcpy (src, twd);
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


static void init_tsh()
{
  getcwd(twd, PATH_MAX);
  char *home = getenv("HOME");
  strcpy(tsh_dir, home);
  strcat(tsh_dir, "/.tsh");
  ret_value = EXIT_SUCCESS;
}

static int special_command(char *s, char **tab, int tab_l, int set_tsh_func)
{
  for (int i = 0; i < tab_l; i++)
    {
      if (strcmp(s, tab[i]) == 0)
	{
	  if (set_tsh_func)
	    {
	      memcpy(tsh_func, tab[i], strlen(tab[i]) + 1);
	    }
	  return 1;
	}
    }
  return 0;
}

static int launch_tsh_func(char **argv, int argc)
{
  if (strcmp(argv[0], "cd") == 0)
    {
      return cd(argv, argc);
    }
  else if (strcmp(argv[0], "exit") == 0)
    {
      exit_tsh(argv, argc);
    }
  else if (strcmp(argv[0], "pwd") == 0)
    {
      return pwd(argv, argc);
    }
  return EXIT_FAILURE;
}

static int count_argc(char **argv)
{
  int i = 0;

  while (argv[i++] != NULL)
    ;

  return i-1;
}

static int pwd(char **argv, int argc)
{
  return
    (write(STDOUT_FILENO, twd, strlen(twd)) < 0 ||
     write(STDOUT_FILENO, "\n", 2) < 0) ?
    EXIT_FAILURE :
    EXIT_SUCCESS;
}

static int exit_tsh(char **argv, int argc)
{
  for (int i = 1; i < argc; i++)
    {
      free(argv[i]);
    }
  free(argv);
  exit(ret_value);
}

static int cd(char **argv, int argc)
{
  if (argc == 1)
    {
      char *home = getenv("HOME");

      if (home == NULL)
	{
	  char *error = "tsh: cd: HOME not set\n";
	  write(STDERR_FILENO, error, strlen(error) + 1);
	  return EXIT_FAILURE;
	}

      if (chdir(home) != 0)
	{
	  return EXIT_FAILURE;
	}

      setenv("PWD", home, 1);
      setenv("OLDPWD", twd, 1);
      memcpy(twd, home, strlen(home) + 1);
      return EXIT_SUCCESS;
    }
  else if (argc == 2)
    {
      if (argv[1][0] == '-') // Option
	{
	  if (argv[1][1] != '\0') // Not supported option
	    {
	      char *err = "tsh: cd: - est la seul option supporté\n";
	      write(STDERR_FILENO, err, strlen(err) + 1);
	      return EXIT_FAILURE;
	    }
	  else // Supported option
	    {
	      free(argv[1]);
	      char *oldpwd = getenv("OLDPWD");
	      if (oldpwd == NULL)
		{
		  char *err = "tsh: cd: \" OLDPWD \" non défini\n";
		  write(STDERR_FILENO, err, strlen(err) + 1);
		  return EXIT_FAILURE;
		}
	      else
		{
		  argv[1] = malloc(PATH_MAX);
		  memcpy(argv[1], oldpwd, strlen(oldpwd) + 1);
		  write(STDOUT_FILENO, oldpwd, strlen(oldpwd));
		  write(STDOUT_FILENO, "\n", 2);
		}
	    }
	}
      else // Not an option
	{
	  char path[PATH_MAX];
	  int arg_len = strlen(argv[1]);
	  if (arg_len + 1 >= PATH_MAX) //too long
	    {
	      return EXIT_FAILURE;
	    }

	  if (argv[1][arg_len-1] != '/')
	    strcat(argv[1], "/");

	  if (!reduce_abs_path(argv[1], path))
	    {
	      error_cmd("tsh: cd", argv[1]);
	      return EXIT_FAILURE;
	    }

	  char *in_tar = split_tar_abs_path(path);

	  if (!in_tar) //No TAR implied
	    {
	      if (chdir(path) != 0)
		{
		  error_cmd("tsh: cd", argv[1]);
		  return EXIT_FAILURE;
		}

	      setenv("PWD", path, 1);
	      setenv("OLDPWD", twd, 1);
	      memcpy(twd, path, strlen(path) + 1);
	    }
	  else //TAR implied
	    {
	      if (*in_tar != '\0') // path inside tar file
		{
		  if (tar_access(path, in_tar, X_OK) != -1) // check execution permission
		    {
		      int in_tar_len = strlen(in_tar);
		      in_tar[in_tar_len-1] = '\0';
		      in_tar[-1] = '/';
		      setenv("PWD", path, 1);
		      setenv("OLDPWD", twd, 1);
		      memcpy(twd, path, strlen(path) + 1);
		      in_tar[-1] = '\0';
		    }
		  else
		    {
		      in_tar[-1] = '/';
		      perror(argv[1]);
		      return EXIT_FAILURE;
		    }
		}
	      else // going into the root of the TAR
		{
		  setenv("PWD", path, 1);
		  setenv("OLDPWD", twd, 1);
		  memcpy(twd, path, strlen(path) + 1);
		}

	      char *before_tar = strrchr(path, '/'); // Not NULL
	      *before_tar = '\0';
	      chdir(path);
	    }
	}
      return EXIT_SUCCESS;
    }
  else
    {
      char *err = "tsh: cd: trop d'arguments\n";
      write(STDERR_FILENO, err, strlen(err));
      return EXIT_FAILURE;
    }
}


int main (int argc, char *argv[])
{
  init_tsh ();

  char *buf;
  char **tokens;
  pid_t cpid;
  int  is_special, wstatus;

  while ((buf = readline(PROMPT)))
    {
      if (!count_words(buf))
	continue;

      add_history(buf);

      tokens = split(buf, &is_special);
      if (is_special == TSH_FUNC)
	{
	  ret_value = launch_tsh_func(tokens, count_argc(tokens));
	}
      else
	{
	  cpid = fork();
	  switch (cpid)
	    {
	    case -1:
	      perror ("tsh: fork");
	      exit (EXIT_FAILURE);

	    case 0: //son
	      if (is_special == TAR_CMD)
		{
		  char cmd_exec[PATH_MAX];
		  strcpy(cmd_exec, tsh_dir);
		  strcat(cmd_exec, "/bin/");
		  strcat(cmd_exec, tokens[0]);
		  ret_value = execv(cmd_exec, tokens);
		}
	      else
		{
		  ret_value = execvp(tokens[0], tokens);
		}

	      if (errno == ENOENT)
		{
		  int size = strlen(tokens[0]) + CMD_NOT_FOUND_SIZE;
		  char error_msg[size];
		  strcpy(error_msg, tokens[0]);
		  strcat(error_msg, CMD_NOT_FOUND);
		  write(STDOUT_FILENO, error_msg, size);
		}

	      exit(EXIT_FAILURE);

	    default: // father
	      wait(&wstatus);
	      ret_value = WEXITSTATUS(wstatus);
	    }

	  if (is_special)
	    {
	      for (int i = 1; tokens[i] != NULL; i++)
		{
		  free(tokens[i]);
		}
	    }
	}
      free(tokens);
      free(buf);
    }

  return 0;
}
