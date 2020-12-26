#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <unistd.h>

#include "tokens.h"

#include "parse_line_test.h"
#include "minunit.h"
#include "tsh_test.h"

#define NB_LINES 8

extern int tests_run;

static char *tokenize_test();
static char *parse_tokens_success_test();
static char *parse_tokens_err_test();

static char *(*tests[])(void) =
{
  tokenize_test,
  parse_tokens_success_test,
  parse_tokens_err_test,
};

static char *all_tests()
{

  for (int i = 0; i < PARSE_LINE_TEST_SIZE; i++)
  {
    mu_run_test(tests[i]);
  }
  return 0;
}

int launch_parse_line_tests()
{
  int prec_tests_run = tests_run;
  char *results = all_tests();
  if (results != 0) {
    printf(RED "%s\n" WHITE, results);
  }
  else {
    printf(GREEN "ALL PARSE_LINE TESTS PASSED\n" WHITE);
  }
  printf("parse_line tests run: %d\n\n", tests_run - prec_tests_run);

  return (results == 0);
}

static char *tokenize_test()
{
  char *line = malloc(PATH_MAX);
  strcpy(line, "< in 2>> err_app       cat | cat | cmd fic >> app 2> err");
  list *tokens = tokenize(line);
  mu_assert("tokenize test: 2 pipes should result with a list of size 3", list_size(tokens) == 3);
  token excpexcted[] =
  {
    {.val.red = STDIN_REDIR, REDIR},
    {.val.arg = "in", ARG},
    {.val.red = STDERR_APPEND, REDIR},
    {.val.arg = "err_app", ARG},
    {.val.arg = "cat", ARG},
    {.type = PIPE},
    {.val.arg = "cat", ARG},
    {.type = PIPE},
    {.val.arg = "cmd", ARG},
    {.val.arg = "fic", ARG},
    {.val.red = STDOUT_APPEND, REDIR},
    {.val.arg = "app", ARG},
    {.val.red = STDERR_REDIR, REDIR},
    {.val.arg = "err", ARG},
    {.type = PIPE}
  };
  token *tok_it;
  array *arr_it = list_remove_first(tokens);
  for (int i = 0; i < 15; i++)
  {
    if (array_size(arr_it) == 0)
    {
      arr_it = list_remove_first(tokens);
    }
    tok_it = array_remove_first(arr_it);
    mu_assert("tokenize test: error on returned tokens type", excpexcted[i].type == tok_it -> type);
    switch(tok_it -> type)
    {
      case ARG:
        mu_assert("Tokenize test: error on return vector elements value (arg)", !strcmp(excpexcted[i].val.arg, tok_it -> val.arg));
        break;
      case REDIR:
        mu_assert("Tokenize test: error on return vector elements value (redir)", excpexcted[i].val.red == tok_it -> val.red);

      default:
        break;
    }
  }
  return 0;
}

static char *parse_tokens_success_test()
{
  char lines[NB_LINES][512] =
  {
    "cmd",
    "cmd | cmd | cmd",
    "cmd | cmd | cmd < fic | cmd",
    "cmd < fic | cmd > fic > fic | cmd ",
    "> fic",
    "cmd < fic >> fic | cmd 2>> fic",
    "cmd > fic",
    "cmd > fic | cmd > fic"
  };
  list *tokens[NB_LINES];
  for (int i = 0; i < NB_LINES; i++)
  {
    tokens[i] = tokenize(lines[i]);
    mu_assert("Parse tokens should return true in parse_tokens_success_test", parse_tokens(tokens[i]));
    list_free(tokens[i], true);
  }
  return 0;
}

static char *parse_tokens_err_test()
{
  char lines[NB_LINES][512] =
  {
    "| cat fic",
    "<",
    "     |",
    "cat fic |",
    "cat fic | cat |    ",
    "cat fic | >",
    "< | cat",
    "< > "
  };
  list *tokens[NB_LINES];
  for (int i = 0; i < NB_LINES; i++)
  {
    tokens[i] = tokenize(lines[i]);
  }

  int null_fd = open("/dev/null", O_WRONLY);
  add_reset_redir(STDERR_FILENO, 0);
  dup2(null_fd, STDERR_FILENO);
  bool all_false[8];
  for (int i = 0; i < NB_LINES; i++)
  {
    all_false[i] = parse_tokens(tokens[i]);
    list_free(tokens[i], true);
  }
  reset_redirs();
  for (int i = 0; i < NB_LINES; i++)
  {
    mu_assert("Parse tokens: should return false in parse_tokens_err_test", !all_false[i]);
  }
  return 0;
}
