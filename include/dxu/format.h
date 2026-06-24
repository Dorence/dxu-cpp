#ifndef DXU_FORMAT_IMPLEMENTATION
#define DXU_FORMAT_IMPLEMENTATION

#include <algorithm>  // std::reverse_copy
#include <cstring>    // memcpy
#include <string>

#include "dxu/version.h"

#ifndef DXU_IS_LITTLE_ENDIAN
#if defined(OS_MACOSX)
#include <machine/endian.h>
#if defined(__DARWIN_LITTLE_ENDIAN) && defined(__DARWIN_BYTE_ORDER)
#define DXU_IS_LITTLE_ENDIAN (__DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN)
#endif
#elif defined(OS_SOLARIS)
#include <sys/isa_defs.h>
#ifdef _LITTLE_ENDIAN
#define DXU_IS_LITTLE_ENDIAN true
#else
#define DXU_IS_LITTLE_ENDIAN false
#endif
#elif defined(OS_FREEBSD) || defined(OS_OPENBSD) || defined(OS_NETBSD) || \
    defined(OS_DRAGONFLYBSD) || defined(OS_ANDROID)
#include <sys/endian.h>
#include <sys/types.h>
#else
#include <endian.h>
#endif
#endif  // DXU_IS_LITTLE_ENDIAN

#ifndef DXU_IS_LITTLE_ENDIAN
#define DXU_IS_LITTLE_ENDIAN (__BYTE_ORDER == __LITTLE_ENDIAN)
#endif  // DXU_IS_LITTLE_ENDIAN

namespace DXU_NAMESPACE::format {

constexpr bool IsLittleEndian = DXU_IS_LITTLE_ENDIAN;
constexpr bool IsBigEndian = !IsLittleEndian;

// Write in little-endian.
inline void PutFixed64(char* out, uint64_t value,
                       int len = sizeof(uint64_t)) noexcept {
  if (IsLittleEndian) {
    memcpy(out, &value, len);
  } else {
    char tmp[sizeof(value)];
    memcpy(tmp, &value, len);
    std::reverse_copy(tmp, tmp + len, out);
  }
}

namespace detail {
// Some codes are inspired by [fmt](https://github.com/fmtlib/fmt).

// Converts value in [0, 100) to char[2].
inline const char* Digit2(size_t value) noexcept {
  alignas(2) static char data[] =
      "0001020304050607080910111213141516171819"
      "2021222324252627282930313233343536373839"
      "4041424344454647484950515253545556575859"
      "6061626364656667686970717273747576777879"
      "8081828384858687888990919293949596979899";
  return &data[value * 2];
}

inline constexpr char NumToHex(uint8_t num) {
  return num < 10 ? ('0' + num) : ('a' + num - 10);
}

// char to "\xhh"
inline std::string CharToHex(uint8_t c) {
  return {'\\', 'x', NumToHex(c >> 4), NumToHex(c & 0x0f)};
}

/**
 * @param escapeQuotes 0 -> don't escape quotes, 1 -> escape ",
 *                     2 -> escape ', 3 -> both " & '
 */
template <class Appendable = std::string>
void ToEscapedString(Appendable& t, char c, int escapeQuotes) {
  if (static_cast<uint8_t>(c) - 0x20u < 0x5fu) {
    // Hot path: printable ASCII [0x20, 0x7e]
    if ((c == '"' && (escapeQuotes & 1)) || (c == '\'' && (escapeQuotes & 2)) ||
        c == '\\') {
      t += '\\';
    }
    t += c;
  } else {
    switch (c) {
      case '\0': t += "\\0"; return;      // 0x00 NUL
      case '\a': t += "\\a"; return;      // 0x07 BEL
      case '\b': t += "\\b"; return;      // 0x08 BS
      case '\t': t += "\\t"; return;      // 0x09 TAB
      case '\n': t += "\\n"; return;      // 0x0a LF
      case '\v': t += "\\v"; return;      // 0x0b VT
      case '\f': t += "\\f"; return;      // 0x0c FF
      case '\r': t += "\\r"; return;      // 0x0d CR
      case '\x1b': t += "\\e"; return;    // 0x1b ESC (\e is GNU extension)
      case '\x7f': t += "\\x7f"; return;  // 0x7f DEL
      default: t += CharToHex(c);         // \x01-\x1f remainder, \x80-\xff
    }
  }
}

}  // namespace detail

// Writes decimal value in [0, 100) to out.
inline void WriteDigit2(char* out, size_t value) noexcept {
  memcpy(out, detail::Digit2(value), 2);
}

inline std::string ToDoubleQuotedString(const std::string& s) {
  std::string ret;
  ret.reserve(s.size() + 2);  // assume all printable
  ret += "\"";
  for (char c : s) {
    detail::ToEscapedString(ret, c, /*escapeQuotes=*/1);
  }
  ret += "\"";
  return ret;
}

}  // namespace DXU_NAMESPACE::format

#endif  // DXU_FORMAT_IMPLEMENTATION
