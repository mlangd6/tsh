#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/limits.h>


#include "tokens.h"
#include "tsh.h"


int exec_line(char *line)
{
  int wstatus, is_special;
  token **tokens;
  char **args;
  int nb_tokens;
  int ret_value;
  pid_t cpid;

  if (!count_words(line))
    return 0;
    
  tokens = tokenize(line, &nb_tokens);
  args = malloc((nb_tokens + 1) * sizeof(char *));
  nb_tokens = exec_tokens(tokens, nb_tokens, args);
  free(tokens);
  if (nb_tokens <= 0) // No tokens or an error occured
  {
    free(line);
    free(args);
    reset_redirs(); // In case some redirections worked
    return 0;
  }
  is_special = special_command(args[0]);
  if (is_special == TSH_FUNC)
  {
    ret_value = launch_tsh_func(args, nb_tokens);
  }
  else
  {
    cpid = fork();
    switch (cpid)
    {
      case -1:
      perror ("tsh: fork");
      exit (EXIT_FAILURE);

      case 0: // child
      if (is_special == TAR_CMD)
      {
        char cmd_exec[PATH_MAX];
        char tsh_dir[PATH_MAX];
        sprintf(cmd_exec, "%s/bin/%s", get_tsh_dir(tsh_dir), args[0]);
        ret_value = execv(cmd_exec, args);
      }
      else
      {
        ret_value = execvp(args[0], args);
      }

      if (errno == ENOENT)
      {
        int size = strlen(args[0]) + CMD_NOT_FOUND_SIZE;
        char error_msg[size];
        strcpy(error_msg, args[0]);
        strcat(error_msg, CMD_NOT_FOUND);
        write(STDOUT_FILENO, error_msg, size);
      }

      exit(EXIT_FAILURE);

      default: // parent
      waitpid(cpid, &wstatus, 0);
      ret_value = WEXITSTATUS(wstatus);
    }

  }
  free(args);
  free(line);
  reset_redirs();
  return ret_value;
}
