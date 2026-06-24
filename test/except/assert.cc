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
    case 0: release_assert(false); break;
    case 1: release_assert(true); break;
    case 2: assert("case 2" == nullptr); break;
    default:
      if (n_case >= 3 && n_case <= 5) {
        release_assert_msg(false, "case %d failed", n_case);
      }
      break;
  }
  printf("done\n");
  return 0;
}
