#include <stdio.h>
#include <string.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/wait.h>

#include "redirection.h"
#include "tsh.h"
#include "path_lib.h"
#include "stack.h"
#include "tar.h"
#include "utils.h"
#include "errors.h"

struct reset_redir {
  int fd;
  int reset_fd;
  pid_t pid;
};

static int get_required_blocks(size_t old_size, size_t read_size, int *padding);
static int redir_append(char *s, int fd, redir_type r);
static int tar_redir_append(char *tar_name, char *in_tar, int fd);
static int stdout_redir(char *s);
static int stderr_redir(char *s);
static int stdout_append(char *s);
static int stderr_append(char *s);
static int stdin_redir(char *s);
static int handle_inside_tar_redir(int fd, char *tar_name, char *in_tar);
static int launch_redir_tar_link(char *tar_name, char *in_tar, redir_type r);
static int append_tar_file(char *tar_name, char *in_tar, int read_fd);
static int handle_outside_tar_redir(int fd, char *filename, int open_flags);
static int handle_inside_tar_stdin_redir(char *tar_name, char *filename);
static int stdin_tar_redir(char *tar_name, char *filename);

stack *reset_fds;

static int (*redirs[])(char *) = {
  stdout_redir,
  stderr_redir,
  stdout_append,
  stderr_append,
  stdin_redir
};

/* Add a reset struct to the stack of the reseter of redirections */
void add_reset_redir(int fd, pid_t pid)
{
  struct reset_redir *reset = malloc(sizeof(struct reset_redir));
  reset -> reset_fd = dup(fd);
  reset -> fd = fd;
  reset -> pid = pid;
  stack_push(reset_fds, reset);
}

/* Init the stack of reset struct */
void init_redirections()
{
  reset_fds = stack_create();
}

void exit_redirections()
{
  reset_redirs(); // In case there is redirections while exiting
  stack_free(reset_fds, true);
}

/* Reset all redirections that has been launched */
void reset_redirs()
{
  while(!stack_is_empty(reset_fds))
  {
    struct reset_redir *reset = stack_pop(reset_fds);
    dup2(reset -> reset_fd, reset -> fd);
    close(reset -> reset_fd);
    if (reset -> pid != 0)
    {
      waitpid(reset -> pid, NULL, 0);
    }
    free(reset);
  }
}

/* Launch redirection of type r to file name arg */
int launch_redir(redir_type r, char *arg)
{
  return redirs[r](arg);
}

/* DEFINITION OF REDIRECTIONS FUNCTIONS BELOW */

/* Function to handle both append redirections */
static int redir_append(char *s, int fd, redir_type r)
{
  if (is_dir_name(s))
  {
    errno = EISDIR;
    error_cmd("tsh", s);
    return -1;
  }
  char path[PATH_MAX];
  if (*s == '/')
    strcpy(path, s);
  else
    sprintf(path, "%s/%s", getenv("PWD"), s);
  char resolved[PATH_MAX];
  if (! reduce_abs_path(path, resolved))
  {
    error_cmd("tsh", path);
    return -1;
  }
  char *in_tar;
  if ((in_tar = split_tar_abs_path(resolved)))
  {
    int link_res = launch_redir_tar_link(resolved, in_tar, r);
    if (link_res != -2) return link_res;
    return tar_redir_append(resolved, in_tar, fd);
  }
  else
  {
    return handle_outside_tar_redir(fd, resolved, O_CREAT | O_APPEND | O_WRONLY);
  }
}

/* Returns the number of required blocks to add content, and set padding to then current padding of file */
static int get_required_blocks(size_t old_size, size_t read_size, int *padding)
{
  *padding = BLOCKSIZE - (old_size % BLOCKSIZE);
  if (*padding == BLOCKSIZE) *padding = 0;
  return read_size <= *padding ? 0 : number_of_block(read_size - *padding);
}

/* Handle main loop for > >> 2> 2>> redirections inside tar */
static int handle_inside_tar_redir(int fd, char *tar_name, char *in_tar)
{
  int pipefd[2];
  if (pipe(pipefd) == -1)
  {
    perror("Redirections: pipe");
    return -1;
  }
  pid_t pid = fork();
  switch(pid)
  {
    case -1:
      perror("Redirections: fork");
      break;
    case 0: // child
    // Le procesus fils lit dans le tube et écrit directement dans le tar
    {
      close(pipefd[1]);
      int ex = append_tar_file(tar_name, in_tar, pipefd[0]) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
      exit(ex);
    }
    default: // parent
      // Ajout du reset et on ferme les deux fd du tube (stdout ou stderr sera
      // redirigé vers le pipe avant de fermé, le tube aura donc toujours un lecteur
      // et un écrivain. c'est ce procesus (père) qui lance ensuite la commande
      add_reset_redir(fd, pid);
      dup2(pipefd[1], fd);
      close(pipefd[1]);
      close(pipefd[0]);
      break;
  }
  return 0;
}

/* Handle >> 2>> redirections inside tar */
static int tar_redir_append(char *tar_name, char *in_tar, int fd)
{
  // On vérifie que la redirection n'est pas vers un dossier
  if (in_tar[0] == '\0')
  {
    errno = EISDIR;
    goto error;
  }
  enum file_type type = type_of_file(tar_name, in_tar, false);
  switch(type)
  {
    case NONE:
      if (is_dir_name(in_tar))
      {
        errno = EISDIR;
        goto error;
      }
      if (errno = ENOENT)
        add_ext_to_tar(tar_name, NULL, in_tar);
      else goto error;
      break;
    case DIR:
      errno = EISDIR;
      goto error;
      break;
    case REG:
      break;
  }
  // On vérifie si le fichier existe déjà et si oui si on peut écrire dedans
  if (tar_access(tar_name, in_tar, W_OK) != 1)
  {
    if (errno == EACCES)
    {
      goto error;
    }
    else if (errno == ENOENT)
    {
      // Si le fichier n'existe pas on lé crée
      add_ext_to_tar(tar_name, NULL, in_tar);
    }
  }
  else // On le déplace à la fin du tar pour éviter tout problème possible avec des lecture sur le même tar en même temps
  {
    move_file_to_end_of_tar(tar_name, in_tar);
  }
  handle_inside_tar_redir(fd, tar_name, in_tar);
  return 0;
  error: {
    char error[PATH_MAX];
    sprintf(error, "%s/%s", tar_name, in_tar);
    error_cmd("tsh", error);
    return -1;
  }
}

/* Launch redirections on linked file if in_tar is a link, else returns -2 */
static int launch_redir_tar_link(char *tar_name, char *in_tar, redir_type r)
{
  int tar_fd = open(tar_name, O_RDONLY);
  if (tar_fd < 0)
    return -1;
  struct posix_header hd;
  seek_header(tar_fd, in_tar, &hd);
  if (hd.typeflag == LNKTYPE || hd.typeflag == SYMTYPE)
  {
    char arg[PATH_MAX];
    if (hd.linkname[0] != '/')
    {
      char *last_slash = strrchr(hd.name, '/');
      if (last_slash)
      {
        last_slash[1] = '\0';
        sprintf(arg, "%s/%s%s", tar_name, hd.name, hd.linkname);
      }
      else
      {
        sprintf(arg, "%s/%s", tar_name, hd.linkname);
      }
    }
    else
    {
      strcpy(arg, hd.linkname);
    }
    close(tar_fd);
    return launch_redir(r, arg);
  }
  close(tar_fd);
  return -2;
}

static int append_tar_file(char *tar_name, char *in_tar, int read_fd)
{
  int tar_fd = open(tar_name, O_RDWR);
  ssize_t read_size;
  char buff[4096];
  struct posix_header hd;
  while(1) {
    // tant que rien n'est lu dans le tube, le processus est en attente
    // Ainsi, read_size == 0 si et seulement si il n'y pas plus d'écrivain,
    // i.e si la redirection est fini
    switch((read_size = read(read_fd, buff, 4096)))
    {
      case -1:
        perror("read on pipe");
        return -1;
      case 0:
        exit(EXIT_SUCCESS);
        break;
      default:
      {
        // On met à jour la taille du header par rapport à read_size
        void update_size(struct posix_header *hd)
        {
          int size = get_file_size(hd);
          long unsigned int new_size = size + read_size;
          sprintf(hd -> size, "%011lo", new_size);
        }
        update_header(&hd, tar_fd, in_tar, update_size);

        // Calcul du décalage nécessaire
        int old_size = get_file_size(&hd) - read_size;
        int padding;
        int required_blocks = get_required_blocks(old_size, read_size, &padding);

        // On crée un espace en blocs pour pouvoir écrire ce qui a été lu
        off_t beg = lseek(tar_fd, old_size, SEEK_CUR);
        off_t tar_size = lseek(tar_fd, 0, SEEK_END);
        if (required_blocks > 0)
        {
          if (fmemmove(tar_fd, beg + padding, tar_size - (beg + padding), beg + padding + required_blocks*BLOCKSIZE)) {
            close(tar_fd);
            return -1;
          }
        }
        // On revient à l'endroit où on veut écrire et on écrit
        lseek(tar_fd, beg, SEEK_SET);
        write(tar_fd, buff, read_size);
        break;
      }
    }
  }
  return 0;

}

static int stdout_redir(char *s)
{
  return 0;
}

static int stderr_redir(char *s)
{
  return 0;
}

static int stdout_append(char *s)
{
  return redir_append(s, STDOUT_FILENO, STDOUT_APPEND);
}

static int stderr_append(char *s)
{
  return redir_append(s, STDERR_FILENO, STDERR_APPEND);
}

static int stdin_redir(char *s)
{
  if (is_dir_name(s))
  {
    errno = EISDIR;
    error_cmd("tsh", s);
    return -1;
  }
  char path[PATH_MAX];
  if (*s == '/')
    strcpy(path, s);
  else
    sprintf(path, "%s/%s", getenv("PWD"), s);
  char resolved[PATH_MAX];
  if (! reduce_abs_path(path, resolved))
  {
    error_cmd("tsh", path);
    return -1;
  }
  char *in_tar;
  if ((in_tar = split_tar_abs_path(resolved)))
  {
    int link_res = launch_redir_tar_link(resolved, in_tar, STDIN_REDIR);
    if (link_res != -2) return link_res;
    return stdin_tar_redir(resolved, in_tar);
  }
  return handle_outside_tar_redir(STDIN_FILENO, resolved, O_RDONLY);
}

static int stdin_tar_redir(char *tar_name, char *filename)
{
  if (filename[0] == '\0')
  {
    errno = EISDIR;
    goto error;
  }
  switch(type_of_file(tar_name, filename, false))
  {
    case NONE:
      goto error;
    case DIR:
      errno = EISDIR;
      goto error;
    case REG:
      break;
  }
  if (tar_access(tar_name, filename, R_OK) != 1)
  {
    goto error;
  }
  return handle_inside_tar_stdin_redir(tar_name, filename);
  error:
  {
    char err[PATH_MAX];
    sprintf(err, "%s/%s", tar_name, filename);
    error_cmd("tsh", err);
    return -1;
  }
}

static int handle_inside_tar_stdin_redir(char *tar_name, char *filename)
{
  int pipefd[2];
  if (pipe(pipefd) == -1)
  {
    perror("Redirections: pipe");
    return -1;
  }
  pid_t pid = fork();
  switch(pid)
  {
    case -1:
      perror("Redirections: fork");
      break;
    case 0: // child
    // Le procesus fils écrit dans le tube
    {
      close(pipefd[0]);
      int tar_fd = open(tar_name, O_RDONLY);
      struct posix_header hd;
      seek_header(tar_fd, filename, &hd);
      ssize_t read_size = get_file_size(&hd);
      read_write_buf_by_buf(tar_fd, pipefd[1], read_size, 4096);
      exit(EXIT_SUCCESS);
    }
    default: // Parent
    {
      add_reset_redir(STDIN_FILENO, pid);
      dup2(pipefd[0], STDIN_FILENO);
      close(pipefd[1]);
      close(pipefd[0]);
      break;
    }
  }
  return 0;
}

static int handle_outside_tar_redir(int fd, char *filename, int open_flags)
{
  struct stat s;
  if (stat(filename, &s) == 0)
  {
    if (S_ISDIR(s.st_mode))
    {
      errno = EISDIR;
      error_cmd("tsh", filename);
      return -1;
    }
  }
  add_reset_redir(fd, 0);
  int new_fd;
  if (O_CREAT & open_flags) new_fd = open(filename, open_flags, 0666);
  else new_fd = open(filename, open_flags);
  if (fd < 0)
  {
    error_cmd("tsh", filename);
    return -1;
  }
  dup2(new_fd, fd);
  close(new_fd);
  return 0;
}
