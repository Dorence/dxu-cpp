#pragma once

#include "catch2/catch_all.hpp"
#include "dxu/version.h"

// about 2.3e-13
#define EPS (std::numeric_limits<double>::epsilon() * 1024.0)

#define REQUIRE_FP_EQ(a, b) \
  REQUIRE_THAT((a), Catch::Matchers::WithinAbs((b), EPS))

#define REQUIRE_FP_NE(a, b) \
  REQUIRE_THAT((a), !Catch::Matchers::WithinAbs((b), EPS))

#define REQUIRE_FP_NEAR(a, b, abs_err) \
  REQUIRE_THAT((a), Catch::Matchers::WithinAbs((b), (abs_err)))

#define REQUIRE_FP_NEAR_REL(a, b, rel_err) \
  REQUIRE_THAT((a), Catch::Matchers::WithinRel((b), (rel_err)))
