#include "dxu/chrono.h"

#include <iostream>

#include "test_common.h"

namespace DXU_NAMESPACE::chrono {

TEST_CASE("Chrono::NowMicros") {
  auto now = NowMicros();
  REQUIRE(now > 0);
  std::cout << "NowMicros(): " << now << std::endl;
};

TEST_CASE("Chrono::NowMicrosSteady") {
  auto now_steady = NowMicrosSteady();
  auto now = NowMicros();
  REQUIRE(now_steady > 0);
  REQUIRE_FP_NEAR_REL(now, now_steady, 0.001);
  std::cout << "NowMicrosSteady(): " << now_steady << " vs NowMicros(): " << now
            << " (diff: " << static_cast<int64_t>(now_steady - now) << ")"
            << std::endl;
};

TEST_CASE("Chrono::NowNanos") {
  auto now = NowNanos();
  REQUIRE(now > 0);
  std::cout << "NowNanos(): " << now << std::endl;
};

TEST_CASE("Chrono::Format") {
  {
    char s[2] = {'\0', '\0'};
    format::WriteDigit2(s, 0);
    REQUIRE((s[0] == '0' && s[1] == '0'));
    format::WriteDigit2(s, 12);
    REQUIRE((s[0] == '1' && s[1] == '2'));
    format::WriteDigit2(s, 99);
    REQUIRE((s[0] == '9' && s[1] == '9'));
  }
  {
    char s[16] = {'\0'};
    detail::WriteTripleDigit2(s, 1, 2, 3, ',');
    REQUIRE(strcmp(s, "01,02,03") == 0);
    detail::WriteTripleDigit2(s, 10, 05, 30, ':');
    REQUIRE(strcmp(s, "10:05:30") == 0);
  }
  {
    // UTC+8
    REQUIRE(TimeToString(1640995204) == "2022/01/01-08:00:04");
    REQUIRE(TimeToString(1773413063) == "2026/03/13-22:44:23");
  }
}

}  // namespace DXU_NAMESPACE::chrono
