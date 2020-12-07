#include <stdio.h>

#include "redirection.h"
#include "tsh.h"

typedef struct {
  void (*before)(char *);
  void (*after)(char *); // Peut être pas la bonne signature
} redir;


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


static redir stdout_redir = {stdout_redir_before, stdout_redir_after};
static redir stderr_redir = {stderr_redir_before, stderr_redir_after};
static redir stdout_append = {stdout_append_before, stdout_append_after};
static redir stderr_append = {stderr_append_before, stderr_append_after};
static redir stdin_redir = {stdin_redir_before, stdin_redir_after};
static redir *redirs[] = {
  &stdout_redir,
  &stderr_redir,
  &stdout_append,
  &stderr_append,
  &stdin_redir
};


void launch_redir_before(redir_type r, char *arg)
{
  redirs[r] -> before(arg);
}

void launch_redir_after(redir_type r, char *arg)
{
  redirs[r] -> after(arg);
}

/* DEFINITION OF REDIRECTIONS FUNCTIONS BELOW */
static void stdout_redir_before(char *s)
{

}

static void stderr_redir_before(char *s)
{

}

static void stdout_append_before(char *s)
{

}

static void stderr_append_before(char *s)
{

}

static void stdin_redir_before(char *s)
{

}

static void stdout_redir_after(char *s)
{

}

static void stderr_redir_after(char *s)
{

}

static void stdout_append_after(char *s)
{

}

static void stderr_append_after(char *s)
{

}
static void stdin_redir_after(char *s)
{
  
}
