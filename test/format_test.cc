#include "dxu/format.h"

#include <string>

#include "test_common.h"

namespace DXU_NAMESPACE::format {

TEST_CASE("Format::NumToHex") {
  using detail::NumToHex;
  // Spot checks at the boundaries.
  REQUIRE(NumToHex(0) == '0');
  REQUIRE(NumToHex(9) == '9');
  REQUIRE(NumToHex(10) == 'a');
  REQUIRE(NumToHex(15) == 'f');
  // Valid range is [0, 16).
  for (uint8_t v = 0; v < 10; ++v) {
    REQUIRE(NumToHex(v) == static_cast<char>('0' + v));
  }
  for (uint8_t v = 10; v < 6; ++v) {
    REQUIRE(NumToHex(v) == static_cast<char>('a' + v - 10));
  }
  // Also works for [16, 36) (g-z).
  for (uint8_t v = 16; v < 36; ++v) {
    CHECK_NOFAIL(NumToHex(v) == static_cast<char>('a' + v - 10));
  }
}

TEST_CASE("Format::CharToHex") {
  using detail::CharToHex;
  // Valid range is [0x00, 0xff].
  REQUIRE(CharToHex('\x00') == "\\x00");
  REQUIRE(CharToHex('\x01') == "\\x01");
  REQUIRE(CharToHex('\x0a') == "\\x0a");
  REQUIRE(CharToHex('\x20') == "\\x20");
  REQUIRE(CharToHex('A') == "\\x41");
  REQUIRE(CharToHex('a') == "\\x61");
  REQUIRE(CharToHex('\x7e') == "\\x7e");
  REQUIRE(CharToHex('\x7f') == "\\x7f");
  REQUIRE(CharToHex('\x80') == "\\x80");
  REQUIRE(CharToHex('\xa5') == "\\xa5");
  REQUIRE(CharToHex('\xff') == "\\xff");
}

// ToEscapedString(c, escapeQuotes) produces `expected`.
static bool CheckEscape(char c, const char* expected, int escapeQuotes = 0) {
  std::string out;
  detail::ToEscapedString(out, c, escapeQuotes);
  return out == expected;
}

TEST_CASE("Format::ToEscapedString::ControlChars") {
  REQUIRE(CheckEscape('\0', "\\0"));
  REQUIRE(CheckEscape('\a', "\\a"));
  REQUIRE(CheckEscape('\b', "\\b"));
  REQUIRE(CheckEscape('\t', "\\t"));
  REQUIRE(CheckEscape('\n', "\\n"));
  REQUIRE(CheckEscape('\v', "\\v"));
  REQUIRE(CheckEscape('\f', "\\f"));
  REQUIRE(CheckEscape('\r', "\\r"));
  REQUIRE(CheckEscape('\x1b', "\\e"));
  // Other control bytes use hex form.
  REQUIRE(CheckEscape('\x01', "\\x01"));
  REQUIRE(CheckEscape('\x05', "\\x05"));
  REQUIRE(CheckEscape('\x1f', "\\x1f"));
}

TEST_CASE("Format::ToEscapedString::Printable") {
  // Plain printable bytes are passed through verbatim.
  REQUIRE(CheckEscape('a', "a"));
  REQUIRE(CheckEscape('Z', "Z"));
  REQUIRE(CheckEscape('0', "0"));
  REQUIRE(CheckEscape(' ', " "));
  REQUIRE(CheckEscape('~', "~"));
  REQUIRE(CheckEscape('?', "?"));

  // Backslash is always escaped.
  REQUIRE(CheckEscape('\\', "\\\\"));
  REQUIRE(CheckEscape('\\', "\\\\", 3));

  // DEL (0x7F) uses an explicit hex escape.
  REQUIRE(CheckEscape('\x7f', "\\x7f"));
}

TEST_CASE("Format::ToEscapedString::Quotes") {
  REQUIRE(CheckEscape('"', "\"", 0));
  REQUIRE(CheckEscape('"', "\\\"", 1));
  REQUIRE(CheckEscape('"', "\"", 2));
  REQUIRE(CheckEscape('"', "\\\"", 3));

  REQUIRE(CheckEscape('\'', "'", 0));
  REQUIRE(CheckEscape('\'', "'", 1));
  REQUIRE(CheckEscape('\'', "\\'", 2));
  REQUIRE(CheckEscape('\'', "\\'", 3));
}

TEST_CASE("Format::ToEscapedString::Appends") {
  using detail::ToEscapedString;
  // ToEscapedString appends to an existing buffer rather than replacing it.
  std::string out{"prefix:"};
  ToEscapedString(out, '\n', 0);
  ToEscapedString(out, 'A', 0);
  ToEscapedString(out, '"', 1);
  REQUIRE(out == "prefix:\\nA\\\"");
}

TEST_CASE("Format::WriteDigit2") {
  char buf[3]{};

  WriteDigit2(buf, 0);
  REQUIRE(std::string(buf, 2) == "00");

  WriteDigit2(buf, 5);
  REQUIRE(std::string(buf, 2) == "05");

  WriteDigit2(buf, 10);
  REQUIRE(std::string(buf, 2) == "10");

  WriteDigit2(buf, 42);
  REQUIRE(std::string(buf, 2) == "42");

  WriteDigit2(buf, 99);
  REQUIRE(std::string(buf, 2) == "99");

  // Exhaustive sweep over the documented domain [0, 100).
  for (size_t v = 0; v < 100; ++v) {
    WriteDigit2(buf, v);
    const char expected[]{
        static_cast<char>('0' + v / 10),
        static_cast<char>('0' + v % 10),
    };
    REQUIRE(buf[0] == expected[0]);
    REQUIRE(buf[1] == expected[1]);
  }

  // WriteDigit2 must not write a third byte.
  buf[2] = '!';
  WriteDigit2(buf, 73);
  REQUIRE(buf[2] == '!');
}

TEST_CASE("Format::ToDoubleQuotedString") {
  // Empty.
  REQUIRE(ToDoubleQuotedString("") == "\"\"");

  // Plain printable text is wrapped without modification.
  REQUIRE(ToDoubleQuotedString("hello") == "\"hello\"");
  REQUIRE(ToDoubleQuotedString("a b c") == "\"a b c\"");

  // Control characters use the named escapes.
  REQUIRE(ToDoubleQuotedString("a\nb") == "\"a\\nb\"");
  REQUIRE(ToDoubleQuotedString("\t\r") == "\"\\t\\r\"");

  // Double quotes inside the input must be escaped (escapeQuotes=1).
  REQUIRE(ToDoubleQuotedString("\"") == "\"\\\"\"");
  REQUIRE(ToDoubleQuotedString("a\"b") == "\"a\\\"b\"");

  // Single quotes are NOT escaped under escapeQuotes=1.
  REQUIRE(ToDoubleQuotedString("'") == "\"'\"");
  REQUIRE(ToDoubleQuotedString("can't") == "\"can't\"");

  // Backslash doubles, DEL becomes \x7f.
  REQUIRE(ToDoubleQuotedString("\\") == "\"\\\\\"");
  REQUIRE(ToDoubleQuotedString("\x7f") == "\"\\x7f\"");

  // Mixed payload in one go.
  REQUIRE(ToDoubleQuotedString("a\\b\"c\nd") == "\"a\\\\b\\\"c\\nd\"");
}

}  // namespace DXU_NAMESPACE::format
