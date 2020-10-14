#include "tar.h"
#include <unistd.h>

#define CMD_NAME "cat"

int main(int argc, char const *argv[]) {
  if (argc == 1) {
    execlp(CMD_NAME, CMD_NAME, NULL);
  }
  return 0;
}
