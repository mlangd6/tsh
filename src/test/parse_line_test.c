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


extern int tests_run;

// static char *tokenize_test();

static char *(*tests[])(void) =
{
  // tokenize_test,
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


// static char *tokenize_test()
// {
  // int nb_el;
  // char *user_input = malloc(PATH_MAX);
  // strcpy(user_input, "< in >   out    cat   2>> err_append");
  // token **tokens = tokenize(user_input, &nb_el);
  // mu_assert("Tokenize test: number of elements is wrong", nb_el == 7);
  // mu_assert("Tokenize test: return value doesn't end by NULL", tokens[nb_el] == NULL);
  // token tests[] =
  // {
    // {.val.red = STDIN_REDIR, REDIR},
    // {.val.arg = "in", ARG},
    // {.val.arg = STDOUT_REDIR, REDIR},
    // {.val.arg = "out", ARG},
    // {.val.arg = "cat", ARG},
    // {.val.red = STDERR_APPEND, REDIR},
    // {.val.arg = "err_append", ARG}
  // };
  // for (int i = 0; i < nb_el; i++)
  // {
    // mu_assert("Tokenize test: error on return vector elements type", tests[i].type == tokens[i] -> type);
    // if (tests[i].type == ARG)
    // {
      // mu_assert("Tokenize test: error on return vector elements value (arg)", !strcmp(tests[i].val.arg, tokens[i] -> val.arg));
    // }
    // else
    // {
      // mu_assert("Tokenize test: error on return vector elements value (redir)", tests[i].val.red == tokens[i] -> val.red);
    // }
    // free(tokens[i]);
  // }
  // free(user_input);
  // free(tokens);
  // return 0;
// }
