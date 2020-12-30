/**
 * @file tsh.h
 * Basic tsh functionnalities and functions.
 */

#ifndef TSH_H
#define TSH_H

#define CMD_NOT_FOUND " : command not found\n"
#define CMD_NOT_FOUND_SIZE 22

#define NB_TAR_CMD 7
#define NB_TSH_FUNC 3
#define TAR_CMD 1
#define TSH_FUNC 2
#define NB_REDIR 5


/**
 * Indicates if a command is a basic command, a tar command or a tsh function.
 * @param s The command.
 * @return 0 if its a basic command, else TSH_FUNC macro or TAR_CMD macro.
 */
int special_command(char *s);

/**
 * Init a variable indicating where to find the tar command files.
 */
void init_tsh_dir();

/**
 * Return the directory where tsh files are installed.
 * @param buf The buf where tsh directory is written
 * @return buf
 */
char *get_tsh_dir(char *buf);

/**
 * Launch a tsh function function such as "cd", "exit" or "pwd".
 * @param argv The vector of arguments of the function.
 * @param argc The number of elements in the vector.
 * @return The return number of the launched function.
 */
int launch_tsh_func(char **argv, int argc);

/**
 * Set the return value of tsh.
 * @param ret The new return value of tsh.
 */
void set_ret_value(int ret);

/**
 * Set a string to the current tsh prompt.
 * @param prompt the string where the prompt is written.
 * @return prompt.
 */
char *set_prompt(char *prompt);



#endif
