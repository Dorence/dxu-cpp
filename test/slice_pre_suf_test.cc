/**
 * @brief Test cases for Slice operations about prefix and suffix.
 * Related APIs:
 * - remove_prefix / remove_suffix
 * - starts_with / ends_with
 * - trim / trim_start / trim_end
 * - strip / strip_start / strip_end
 */

#include "dxu/slice.h"
#include "test_common.h"

namespace DXU_NAMESPACE {

TEST_CASE("Slice::RemovePrefix") {
  Slice s("hello");
  s.remove_prefix(2);
  REQUIRE(s == "llo");
  s.remove_prefix_s(100);
  REQUIRE(s.empty());
}

TEST_CASE("Slice::RemoveSuffix") {
  Slice s("hello");
  s.remove_suffix(2);
  REQUIRE(s == "hel");
  s.remove_suffix_s(100);
  REQUIRE(s.empty());
}

TEST_CASE("Slice::StartsWith") {
  Slice s("hello");
  Slice s2("helloworld", 7);
  REQUIRE(s.starts_with("he"));
  REQUIRE(s.starts_with("hello"));
  REQUIRE_FALSE(s.starts_with("hex"));
  REQUIRE(s2.starts_with(s));
  REQUIRE_FALSE(s.starts_with(s2));

  Slice s3{"bad"};
  REQUIRE(s3.starts_with(Slice("")));
  REQUIRE(s3.starts_with(Slice(nullptr)));
  REQUIRE_FALSE(Slice().starts_with("a"));
}

TEST_CASE("Slice::EndsWith") {
  Slice s("hello");
  REQUIRE(s.ends_with("lo"));
  REQUIRE(s.ends_with("hello"));
  REQUIRE_FALSE(s.ends_with("lx"));
  REQUIRE(s.ends_with(Slice("")));
  REQUIRE(s.ends_with(Slice(nullptr)));
  REQUIRE_FALSE(Slice("").ends_with("a"));

  Slice s2("helloworld", 7);  // "hellowo"
  REQUIRE(s2.ends_with("owo"));
  REQUIRE_FALSE(s2.ends_with("world"));
}

TEST_CASE("Slice::Trim") {
  Slice s("  hello  \t");
  REQUIRE(s.trim() == "hello");
  REQUIRE(s.trim_start() == Slice(s.data() + 2, 8));
  REQUIRE(s.trim_end() == "  hello");
  s.remove_prefix_s(3);
  REQUIRE(s.trim() == "ello");
}

TEST_CASE("Slice::Strip") {
  Slice s("  hello  \t");
  REQUIRE(s.strip(" \t") == "hello");
  REQUIRE(s.strip(" ") == "hello  \t");
  REQUIRE(s.strip("\t") == "  hello  ");
  REQUIRE(s.strip_start(" he") == "llo  \t");
  REQUIRE(s.strip_end(" \t") == "  hello");

  REQUIRE(s.strip("\n\r") == s);
  REQUIRE(s.strip(" \thelo").empty());
};

}  // namespace DXU_NAMESPACE
