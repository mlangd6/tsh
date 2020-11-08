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

#define PROMPT "$ "
#define CMD_NOT_FOUND " : command not found\n"
#define CMD_NOT_FOUND_SIZE 22
#define NB_TAR_CMD 1

static int count_words(char *s);
static char **split(char *user_input, int *is_tar_cmd);
static void init_tsh();
static int is_tar_command(char *s);

char tsh_dir[PATH_MAX];
char *tar_cmds[NB_TAR_CMD] = {"cat"}; //"ls"};
char twd[PATH_MAX];

static int count_words(char *s) {
  int res = 1;
  char *chr = s;
  while((chr = strchr(chr, ' ')) != NULL && chr[1] != '\0' && res++) {
    chr++;
  }
  return res;
}

static char **split(char *user_input, int *is_tar_cmd)
{
  const char delim[] = " ";
  int nb_tokens = count_words(user_input);
  char **res = malloc((nb_tokens+1) * sizeof(char *)); // +1 pour le NULL à la fin
  char *tok, *src;
  
  res[0] = strtok(user_input, delim);
  *is_tar_cmd = is_tar_command(res[0]);
  
  for (int i = 1; i < nb_tokens; i++)
    {
      tok = strtok(NULL, delim);

      if (*is_tar_cmd)
	{	  	  
	  if (tok[0] != '/') // Relative path
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

static void init_tsh() {
  getcwd(twd, PATH_MAX);
  char *home = getenv("HOME");
  strcpy(tsh_dir, home);
  strcat(tsh_dir, "/.tsh");
}

static int is_tar_command(char *s) {
  for (int i = 0; i < NB_TAR_CMD; i++) {
    if (strcmp(s, tar_cmds[i]) == 0) {
      return 1;
    }
  }
  return 0;
}

int main (int argc, char *argv[]) {
  init_tsh ();

  char *buf;
  char **tokens;
  int p, w, is_tar_cmd;
  
  while ((buf = readline(PROMPT))) {

    if (strlen(buf) > 0){
      add_history(buf);
    }

    tokens = split(buf, &is_tar_cmd);
    
    switch (p = fork()) {
      case -1:
        perror("tsh: fork");
        exit(EXIT_FAILURE);
      case 0: //son
        if (is_tar_command(tokens[0])) {
          char cmd_exec[PATH_MAX];
          strcpy(cmd_exec, tsh_dir);
          strcat(cmd_exec, "/bin/");
          strcat(cmd_exec, tokens[0]);
          execv(cmd_exec, tokens);
        }
        else {
          execvp(tokens[0], tokens);
        }

	if (errno == ENOENT) {
          int size = strlen(tokens[0]) + CMD_NOT_FOUND_SIZE;
          char error_msg[size];
          strcpy(error_msg, tokens[0]);
          strcat(error_msg, CMD_NOT_FOUND);
          write(STDOUT_FILENO, error_msg, size);
        }
        exit(EXIT_FAILURE);
	
      default: // father
        wait(&w);
    }
    if (is_tar_cmd) {
      for (int i = 1; tokens[i] != NULL; i++) {
        free(tokens[i]);
      }
    }
    free(tokens);
    free(buf);
  }
  
  return 0;
}
