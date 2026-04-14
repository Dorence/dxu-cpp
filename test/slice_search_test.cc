/**
 * @brief Test cases for Slice search operations.
 * Related APIs:
 * - find / rfind / contains
 * - find_first_of / find_last_of / find_first_not_of / find_last_not_of
 */

#include "dxu/slice.h"
#include "test_common.h"

namespace DXU_NAMESPACE {

// try best to fit std::string behavior in corner cases
TEST_CASE("Slice::Find") {
  Slice s{"hello", 5};

  /* empty full slice */
  REQUIRE(std::string{}.find("") == 0);
  REQUIRE(Slice{}.find("") == 0);
  // std::string{}.find(nullptr) -> SIGTRAP
  REQUIRE(Slice{}.find(Slice::Invalid()) == Slice::npos);
  /* empty sub slice */
  REQUIRE(std::string{"hello"}.find("") == 0);
  REQUIRE(s.find("") == 0);
  REQUIRE(s.find(Slice{}) == 0);
  /* invalid full slice */
  REQUIRE(Slice::Invalid().find("") == Slice::npos);
  REQUIRE(Slice::Invalid().find(Slice::Invalid()) == Slice::npos);
  /* invalid sub slice */
  // std::string{"hello"}.find(nullptr) -> SIGTRAP
  REQUIRE(s.find(Slice::Invalid()) == Slice::npos);

  REQUIRE(s.find("l") == 2);
  REQUIRE(s.find("lo", 3) == 3);
  REQUIRE(s.find("hello") == 0);
  REQUIRE(s.find("hello", 1) == Slice::npos);
  REQUIRE(s.find("bla") == Slice::npos);

  REQUIRE(s.find('l') == 2);
  REQUIRE(s.find('l', 3) == 3);
  REQUIRE(s.find('l', 4) == Slice::npos);
  REQUIRE(s.find('z') == Slice::npos);
}

TEST_CASE("Slice::FindFirst") {
  Slice s{"hello", 5};

  /* empty full slice */
  REQUIRE(std::string{""}.find_first_of("") == std::string::npos);
  REQUIRE(Slice{}.find_first_of("") == Slice::npos);
  REQUIRE(std::string{""}.find_first_of(nullptr) == std::string::npos);
  REQUIRE(Slice{}.find_first_of(Slice::Invalid()) == Slice::npos);
  /* invalid full slice */
  REQUIRE(Slice::Invalid().find_first_of("") == Slice::npos);
  REQUIRE(Slice::Invalid().find_first_of(Slice::Invalid()) == Slice::npos);
  /* empty sub slice */
  REQUIRE(std::string{"hello"}.find_first_of("") == std::string::npos);
  REQUIRE(s.find_first_of("") == std::string::npos);
  /* invalid sub slice */
  // std::string{"hello"}.find_first_of(nullptr) -> SIGSEGV
  REQUIRE(s.find_first_of(Slice::Invalid()) == Slice::npos);

  REQUIRE(s.find_first_of("h") == 0);
  REQUIRE(s.find_first_of("l") == 2);
  REQUIRE(s.find_first_of("o") == 4);
  REQUIRE(s.find_first_of("lo") == 2);   // find 'l'
  REQUIRE(s.find_first_of("efg") == 1);  // find 'e'
  REQUIRE(s.find_first_of("xyz") == Slice::npos);

  /* empty full slice */
  REQUIRE(std::string{""}.find_first_not_of("") == std::string::npos);
  REQUIRE(Slice{}.find_first_not_of("") == Slice::npos);
  REQUIRE(std::string{""}.find_first_not_of(nullptr) == std::string::npos);
  REQUIRE(Slice{}.find_first_not_of(Slice::Invalid()) == Slice::npos);
  /* invalid full slice */
  REQUIRE(Slice::Invalid().find_first_not_of("") == Slice::npos);
  REQUIRE(Slice::Invalid().find_first_not_of(Slice::Invalid()) == Slice::npos);
  /* empty sub slice */
  REQUIRE(std::string{"hello"}.find_first_not_of("") == 0);
  REQUIRE(s.find_first_not_of("") == 0);  // find 'h'
  /* invalid sub slice */
  // std::string{"hello"}.find_first_not_of(nullptr) -> SIGSEGV
  REQUIRE(s.find_first_not_of(Slice::Invalid()) == Slice::npos);

  REQUIRE(s.find_first_not_of("abc") == 0);  // find 'h'
  REQUIRE(s.find_first_not_of("h") == 1);    // find 'e'
  REQUIRE(s.find_first_not_of("hel") == 4);  // find 'o'
  REQUIRE(s.find_first_not_of("ehlo") == Slice::npos);
}

TEST_CASE("Slice::Rfind") {
  Slice s{"hello", 5};

  /* empty full slice */
  REQUIRE(std::string{""}.rfind('x') == std::string::npos);
  REQUIRE(Slice{}.rfind('x') == Slice::npos);
  /* invalid full slice */
  REQUIRE(Slice::Invalid().rfind('x') == Slice::npos);
  /* null char */
  REQUIRE(std::string{"hello"}.rfind('\0') == std::string::npos);
  REQUIRE(s.rfind('\0') == Slice::npos);

  REQUIRE(s.rfind('h') == 0);
  REQUIRE(s.rfind('h', 0) == 0);
  REQUIRE(s.rfind('h', 1) == 0);
  REQUIRE(s.rfind('l') == 3);  // second 'l'
  REQUIRE(s.rfind('l', 1) == Slice::npos);
  REQUIRE(s.rfind('l', 2) == 2);    // first 'l'
  REQUIRE(s.rfind('l', 100) == 3);  // second 'l'
  REQUIRE(s.rfind('o') == 4);
  REQUIRE(s.rfind('z') == Slice::npos);
}

TEST_CASE("Slice::FindLast") {
  Slice s{"hello", 5};

  /* empty sub slice */
  REQUIRE(std::string{"hello"}.find_last_of("") == std::string::npos);
  REQUIRE(s.find_last_of("") == Slice::npos);

  REQUIRE(s.find_last_of("h") == 0);
  REQUIRE(s.find_last_of("e") == 1);
  REQUIRE(s.find_last_of("o") == 4);
  REQUIRE(s.find_last_of("z") == Slice::npos);
  REQUIRE(s.find_last_of("abcde") == 1);  // find 'e'
  REQUIRE(s.find_last_of("lo") == 4);     // find 'o'
  REQUIRE(s.find_last_of("xyz") == Slice::npos);

  /* empty sub slice */
  REQUIRE(std::string{"hello"}.find_last_not_of("") == 4);
  REQUIRE(s.find_last_not_of("") == 4);
  /* invalid sub slice */
  // std::string{"hello"}.find_last_not_of(nullptr) -> SIGTRAP
  REQUIRE(s.find_last_not_of(Slice::Invalid()) == Slice::npos);

  REQUIRE(s.find_last_not_of("abc") == 4);  // find 'o'
  REQUIRE(s.find_last_not_of("opq") == 3);  // second 'l'
  REQUIRE(s.find_last_not_of("eol") == 0);  // find 'h'
  REQUIRE(s.find_last_not_of("ehlo") == Slice::npos);
}

}  // namespace DXU_NAMESPACE
