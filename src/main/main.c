#include <stdio.h>
#include "tar.h"
#include "ls.h"
int main(int argc, char *argv[]){
  ls_l("/tmp/tsh_test/test.tar");
  printf("Hello World!\n");
  return 0;
}
