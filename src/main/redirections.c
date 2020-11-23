#include <stdio.h>

#include "tsh.h"

static void stdout_redir_before(char *s);
static void stderr_redir_before(char *s);
static void stdout_append_before(char *s);
static void stderr_append_before(char *s);
static void stdin_redir_before(char *s);
static void stdout_redir_after(char *s);
static void stderr_redir_after(char *s);
static void stdout_append_after(char *s);
static void stderr_append_after(char *s);
static void stdin_redir_after(char *s);


redir_type *redirs[NB_REDIR];
redir_type stdout_redir = {">", stdout_redir_before, stdout_redir_after};
redir_type stderr_redir = {"2>", stderr_redir_before, stderr_redir_after};
redir_type stdout_append = {">>", stdout_append_before, stdout_append_after};
redir_type stderr_append = {"2>>", stderr_append_before, stderr_append_after};
redir_type stdin_redir = {"<", stdin_redir_before, stdin_redir_after};

static void stdout_redir_before(char *s)
{
  printf("redir stdout to %s, before function not implemented\n", s);
}

static void stderr_redir_before(char *s)
{
  printf("redir stderr to %s, before function not implemented\n", s);
}

static void stdout_append_before(char *s)
{
  printf("redir append stdout to %s, before function not implemented\n", s);
}

static void stderr_append_before(char *s)
{
  printf("redir append stderr to %s, before function not implemented\n", s);
}

static void stdin_redir_before(char *s)
{
  printf("redir append stdout to %s, before function not implemented\n", s);
}

static void stdout_redir_after(char *s)
{
  printf("redir stdout to %s, after function not implemented\n", s);
}

static void stderr_redir_after(char *s)
{
  printf("redir stdout to %s, after function not implemented\n", s);
}

static void stdout_append_after(char *s)
{
  printf("redir append stdout to %s, after function not implemented\n", s);
}

static void stderr_append_after(char *s)
{
  printf("redir append stderr to %s, after function not implemented\n", s);
}
static void stdin_redir_after(char *s)
{
  printf("redir stdin to %s, after function not implemented\n", s);
}
