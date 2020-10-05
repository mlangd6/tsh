#include "minunit.h"
#include "tsh_test.h"
#include "tar_test.h"
#include <stdlib.h>
#include <stdio.h>



void before() {
  system("./create_test_tar.sh");
}

int main(int argc, char const *argv[]) {
  int ret = 1;
  ret = ret && launch_tar_tests();
  return ret;
}
