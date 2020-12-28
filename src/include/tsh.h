#ifndef TSH_H
#define TSH_H

#define PROMPT "$ "
#define CMD_NOT_FOUND " : command not found\n"
#define CMD_NOT_FOUND_SIZE 22

#define NB_TAR_CMD 7
#define NB_TSH_FUNC 3
#define TAR_CMD 1
#define TSH_FUNC 2
#define NB_REDIR 5



int special_command(char *s);
void init_tsh_dir();
char *get_tsh_dir(char *buf);
int launch_tsh_func(char **argv, int argc);
void set_ret_value(int ret);



#endif
