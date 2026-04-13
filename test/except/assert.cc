#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "dxu/release_assert.h"

int main(int argc, char* argv[]) {
  int n_case = 0;
  if (argc > 1) {
    n_case = atoi(argv[1]);
  }
  switch (n_case) {
    case 0:
      release_assert(false);
      break;
    case 1:
      release_assert(true);
      break;
    case 2:
      assert("case 2" == nullptr);
      break;
    case 3:
    case 4:
    case 5:
      release_assert_msg(false, "case %d failed", n_case);
      break;
    default:
      break;
  }
  printf("done\n");
  return 0;
}
