#include "minunit.h"
#include "tsh_test.h"
#include <stdlib.h>
#include <stdio.h>

void before() {
  system("./create_test_tar.sh");
}

int main(int argc, char const *argv[]) {
  printf("Hello World!\n");
  return 0;
}
