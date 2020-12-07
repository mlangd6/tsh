#include <string.h>

#include "tsh.h"

const char *tar_cmds[NB_TAR_CMD] = {"cat", "ls"};
const char *tsh_funcs[NB_TSH_FUNC] = {"cd", "exit", "pwd"};

int special_command(char *s)
{
  int max = (NB_TAR_CMD > NB_TSH_FUNC) ? NB_TAR_CMD : NB_TSH_FUNC;
  for (int i = 0; i < max; i++)
  {
    if (i < NB_TAR_CMD && strcmp(s, tar_cmds[i]) == 0)
    {
      return TAR_CMD;
    }
    if (i < NB_TSH_FUNC && strcmp(s, tsh_funcs[i]) == 0)
    {
      return TSH_FUNC;
    }
  }
  return 0;
}
