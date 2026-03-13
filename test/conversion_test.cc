#include "dxu/conversion.h"

#include "test_common.h"

namespace DXU_NAMESPACE {

struct Point {
  int x;
  int y;
};

namespace detail {
template <>
inline void VectorToStringFormat(std::string& r, const Point& e) {
  r += '(' + std::to_string(e.x) + ',' + std::to_string(e.y) + ')';
}
}  // namespace detail

TEST_CASE("Conversion::StrToInt") {
  REQUIRE(StrToInt("123") == 123);
  REQUIRE(StrToInt("1") == 1);
  REQUIRE(StrToInt("-0") == 0);
  REQUIRE(StrToInt("-4") == -4);

  int err = 0;
  StrToIntOptions opts = {&err, 0, false, true};
  REQUIRE(StrToInt("100", opts) == 100);
  REQUIRE(err == 0);
  REQUIRE(StrToInt("10+0", opts) == 0);
  REQUIRE(err == EILSEQ);
  REQUIRE(StrToInt("100 ", opts) == 0);
  REQUIRE(err == EILSEQ);
  REQUIRE(StrToInt("</>", opts) == 0);
  REQUIRE(err == EINVAL);
};

TEST_CASE("Conversion::VectorToString") {
  REQUIRE(VectorToString(std::vector<int>{1, 2, 3}) == "[1, 2, 3]");
  // treat char as integral type
  REQUIRE(VectorToString(std::vector<char>{'a'}) == "[97]");
  REQUIRE(VectorToString(std::vector<int64_t>{-2, 0, 3}, ";", "<", ">") ==
          "<-2;0;3>");
  REQUIRE(VectorToString(std::vector<double>{2.33, 0.0, 1.0}, ":", "", "") ==
          "2.330000:0.000000:1.000000");
  REQUIRE(VectorToString(std::vector<std::string>{"a", "b", "c"}) ==
          "[a, b, c]");
  REQUIRE(VectorToString(std::vector<std::string>{"a", "", "z"}, "\", \"",
                         "{\"", "\"}") == "{\"a\", \"\", \"z\"}");
  // custom type
  REQUIRE(VectorToString(std::vector<Point>{{1, 2}, {3, 4}}) ==
          "[(1,2), (3,4)]");
};

TEST_CASE("Conversion::StringToVectorInt") {
  using vint = std::vector<int>;
  vint result;

  result = StringToVectorInt("");
  REQUIRE(result.size() == 0);
  result = StringToVectorInt(",");
  REQUIRE(result.size() == 0);
  result = StringToVectorInt("1");
  REQUIRE(result == (vint{1}));
  result = StringToVectorInt("1*1");
  REQUIRE(result == (vint{1}));
  result = StringToVectorInt("1*3");
  REQUIRE(result == vint(3, 1));
  result = StringToVectorInt("1,3");
  REQUIRE(result == (vint{1, 3}));
  result = StringToVectorInt("1,3,6,");
  REQUIRE(result == (vint{1, 3, 6}));
  result = StringToVectorInt("1,3,6,  ");
  REQUIRE(result == (vint{1, 3, 6}));
  result = StringToVectorInt("1-3");
  REQUIRE(result == (vint{1, 2, 3}));
  result = StringToVectorInt("-1*2,3*4");
  REQUIRE(result == (vint{-1, -1, 3, 3, 3, 3}));
  result = StringToVectorInt(" -1-2 , 5*1 , -1--2, 1-1");
  REQUIRE(result == (vint{-1, 0, 1, 2, 5, -1, -2, 1}));
  result = StringToVectorInt("12345, 0, -12345,-2 , 1 ");
  REQUIRE(result == (vint{12345, 0, -12345, -2, 1}));
};

TEST_CASE("Conversion::StringToVectorIntAdvanced") {
  using vint = std::vector<int>;
  vint result;
  int err;
  StringToVectorIntOptions opts = {&err, 0, ',', '*', '-'};
  {
    opts.sep = ' ';
    result = StringToVectorInt("12345 0 -12345", opts);
    REQUIRE(err == 0);
    REQUIRE(result == (vint{12345, 0, -12345}));
  }
  {
    opts.sep = ' ';
    result = StringToVectorInt("12345  0   -12345 \t5", opts);
    REQUIRE(err == 0);
    REQUIRE(result == (vint{12345, 0, -12345, 5}));
  }
  {
    opts.sep = ';';
    result = StringToVectorInt("1 ; 0 ; -1--1 ;", opts);
    REQUIRE(err == 0);
    REQUIRE(result == (vint{1, 0, -1}));
  }
  {
    opts.sep = ',';
    opts.range = ':';
    result = StringToVectorInt("\t1   : 4  ", opts);
    REQUIRE(err == 0);
    REQUIRE(result == (vint{1, 2, 3, 4}));
  }
  {
    vint expected;
    expected.reserve(10000);
    for (int i = 0; i < 10000; i++) {
      expected.push_back(i);
    }
    opts.range = '-';
    result = StringToVectorInt("0-9999", opts);
    REQUIRE(result == expected);
    REQUIRE(err == 0);
  }
};

TEST_CASE("Conversion::StringToVectorIntError") {
  using vint = std::vector<int>;
  vint result;
  int err;
  StringToVectorIntOptions opts = {&err, 0, ',', '*', '-'};
  result = StringToVectorInt("-1*", opts);
  REQUIRE(err != 0);
  REQUIRE(result.size() == 0);
  result = StringToVectorInt("-1+", opts);
  REQUIRE(err != 0);
  REQUIRE(result.size() == 0);
  result = StringToVectorInt(",,,", opts);
  REQUIRE(err != 0);
  REQUIRE(result.size() == 0);
  result = StringToVectorInt("-9,\t100.0,  200", opts);
  REQUIRE(err != 0);
  REQUIRE(result.size() == 1);
  result = StringToVectorInt("100*10,1*-1", opts);
  REQUIRE(err != 0);
  REQUIRE(result.size() == 10);
  result = StringToVectorInt("233,abc,", opts);
  REQUIRE(err != 0);
  REQUIRE(result.size() == 1);

  opts.sep = '\t';
  result = StringToVectorInt("12345\t0-2   -12345", opts);
  REQUIRE(err == EILSEQ);
  REQUIRE(result == (vint{12345}));
};

}  // namespace DXU_NAMESPACE
