#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <unistd.h>

#include "parse_line.h"

#include "minunit.h"
#include "tsh_test.h"

#define PARSE_LINE_TEST_SIZE 2

extern int tests_run;

static char *tokenize_test();
static char *exec_tokens_test();

static char *(*tests[])(void) =
{
  tokenize_test,
  exec_tokens_test
};

static char *tokenize_test()
{
  int nb_el;
  char *user_input = malloc(PATH_MAX);
  strcpy(user_input, "< in >   out    cat   2>> err_append");
  token **tokens = tokenize(user_input, &nb_el);
  mu_assert("Tokenize test: number of elements is wrong", nb_el == 7);
  mu_assert("Tokenize test: return value doesn't end by NULL", tokens[nb_el] == NULL);
  token tests[] =
  {
    {.val.red = STDIN_REDIR, REDIR},
    {.val.arg = "in", ARG},
    {.val.arg = STDOUT_REDIR, REDIR},
    {.val.arg = "out", ARG},
    {.val.arg = "cat", ARG},
    {.val.red = STDERR_APPEND, REDIR},
    {.val.arg = "err_append", ARG}
  };
  for (int i = 0; i < nb_el; i++)
  {
    mu_assert("Tokenize test: error on return vector elements type", tests[i].type == tokens[i] -> type);
    if (tests[i].type == ARG)
    {
      mu_assert("Tokenize test: error on return vector elements value (arg)", !strcmp(tests[i].val.arg, tokens[i] -> val.arg));
    }
    else
    {
      mu_assert("Tokenize test: error on return vector elements value (redir)", tests[i].val.red == tokens[i] -> val.red);
    }
    free(tokens[i]);
  }
  free(user_input);
  free(tokens);
  return 0;
}

static char *exec_tokens_test()
{
  int nb_el;
  char *user_input = malloc(PATH_MAX);
  strcpy(user_input, "< in >   out    cat   fic 2>> err_append");
  token **tokens = tokenize(user_input, &nb_el);
  char **argv = malloc((nb_el + 1) * sizeof(char *));
  nb_el = exec_tokens(tokens, nb_el, argv);
  mu_assert("Exec tokens test: error on size of argv", nb_el == 2);
  char *tests[] = {"cat", "fic"};
  mu_assert("Exec tokens test: argv[0] != \"cat\"", !strcmp(argv[0], tests[0]));
  mu_assert("Exec tokens test: argv[2] != \"fic\"", !strcmp(argv[1], tests[1]));
  mu_assert("Exec tokens test: argv[2] != NULL", argv[2] == NULL);
  free(tokens);
  free(user_input);
  free(argv);
  return 0;
}


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
    printf("%s\n", results);
  }
  else {
    printf("ALL PATH_LIB TESTS PASSED\n");
  }
  printf("path_lib tests run: %d\n\n", tests_run - prec_tests_run);

  return (results == 0);
}
