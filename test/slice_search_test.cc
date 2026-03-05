/**
 * @brief Test cases for Slice search operations.
 * Related APIs:
 * - find / rfind / contains
 * - find_first_of / find_last_of / find_first_not_of / find_last_not_of
 */

#include "dxu/slice.h"
#include "test_common.h"

namespace DXU_NAMESPACE {

TEST_CASE("Slice::Find") {
  Slice s{"hello", 5};
  // REQUIRE(std::string{"hello"}.find("") == 0);
  REQUIRE(s.find("") == 0);
  REQUIRE(s.find("l") == 2);
  REQUIRE(s.find("lo", 3) == 3);
  REQUIRE(s.find("hello") == 0);
  REQUIRE(s.find("hello", 1) == Slice::npos);

  REQUIRE(s.find('l') == 2);
  REQUIRE(s.find('l', 3) == 3);
  REQUIRE(s.find('l', 4) == Slice::npos);
  REQUIRE(s.find('z') == Slice::npos);
}

}  // namespace DXU_NAMESPACE
