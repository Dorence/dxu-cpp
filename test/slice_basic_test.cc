/**
 * @brief Test cases for Slice basic operations.
 * Related APIs:
 * - constructors
 * - size / data / empty / valid
 * - operator[]
 * - ToString / ToStringView / DecodeHex
 * - clear
 * - compare / operator== / operator!=
 */

#include "dxu/slice.h"
#include "test_common.h"

namespace DXU_NAMESPACE {

TEST_CASE("Slice::Empty") {
  // default ctor
  REQUIRE(Slice().empty());
  REQUIRE(Slice().size() == 0);

  // empty string
  Slice s("");
  REQUIRE(s.empty());
  REQUIRE(strcmp(s.data(), "") == 0);
  REQUIRE(s.valid());

  // nullptr
  REQUIRE(Slice(nullptr).empty());
  REQUIRE(Slice(nullptr).size() == 0);
  REQUIRE(Slice(nullptr).data() == nullptr);
  REQUIRE_FALSE(Slice(nullptr).valid());
  REQUIRE_FALSE(Slice::Invalid().valid());

  // empty equals
  REQUIRE(Slice() == Slice(nullptr));
  REQUIRE(Slice("") == Slice(nullptr));

  // cleared slice
  s = "123456";
  s.clear();
  REQUIRE(s.empty());
  REQUIRE(s.valid());
};

TEST_CASE("Slice::Equals") {
  Slice s("123456");
  REQUIRE(s == "123456");
  REQUIRE(s == Slice("123456"));
  REQUIRE(s == Slice("123456abcd", 6));
  REQUIRE(s != "1234567");
  REQUIRE(s != Slice("123456abcd"));
};

TEST_CASE("Slice::ToString") {
  std::string s = "123456";
  REQUIRE(s == Slice(s).ToString());
  REQUIRE(Slice(s).ToString(true) == "313233343536");
};

TEST_CASE("Slice::DecodeHex") {
  std::string buf;
  REQUIRE(Slice("6c6F7665").DecodeHex(&buf));
  REQUIRE(buf == "love");
};

}  // namespace DXU_NAMESPACE
