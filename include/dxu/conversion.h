#ifndef DXU_CONVERSION_H_INCLUDE
#define DXU_CONVERSION_H_INCLUDE

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "dxu/version.h"

namespace DXU_NAMESPACE {

// remove ' ' and '\t' from both ends
void TrimString(std::string& str);

// any invalid format causes error
int StrToIntSafe(const char* str, int32_t& result, const int& base = 0);

// any invalid format causes error
int StrToInt64Safe(const char* str, int64_t& result, const int& base = 0);

// @warn str trims ending in-place
int StrToIntWithTrim(std::string& str, int32_t& result, size_t front = 0);

// @warn str trims ending in-place
int StrToInt64WithTrim(std::string& str, int64_t& result, size_t front = 0);

// parse "a:b", @warn str trims ending in-place
int StrToInt32PairWithTrim(std::string& str,
                           std::pair<int32_t, int32_t>& result,
                           size_t front = 0, const char& sep = ':');

enum BytesUnit : uint8_t { SizeByte, SizeKB, SizeMB, SizeGB, SizeTB, SizePB };

// size <= 7 (16384PB)
BytesUnit HumanReadableBytes(uint64_t bytes, std::string& result);

/**
 * Features:
 * - Accepts spaces ' ' and trailing ',': `"0  ,  0,0,"` -> [0, 0, 0]
 * - '*' for repeat: `"1*3,8,-8,4*2"` -> [1, 1, 1, 8, -8, 4, 4]
 * - '-' for range:  `"1-3,0,2--1"` -> [1, 2, 3, 0, 2, 1, 0, -1]
 * - Custom seperator: `"1 2 3" (sep=' ')` -> [1, 2, 3]
 *   If sep is `' '`, doesn't accept continued space: `"1  2"` -> [1] (err>0)
 */
std::vector<int> StringToVectorInt(const std::string& str, int* err = nullptr,
                                   const char& sep = ',');

namespace detail {
template <class T>
inline typename std::enable_if<std::is_arithmetic<T>::value>::type
VectorToStringFormat(std::string& r, const T& e) {
  r += std::to_string(e);
}

template <class T>
inline typename std::enable_if<!std::is_arithmetic<T>::value>::type
VectorToStringFormat(std::string& r, const T& e) {
  r += e;
}
}  // namespace detail

template <class T>
std::string VectorToString(const std::vector<T>& vec,
                           const std::string& sep = ", ",
                           const std::string& start = "[",
                           const std::string& end = "]") {
  constexpr size_t kMeanElementLength = 3;
  std::string r;
  r.reserve(start.size() + vec.size() * (sep.size() + kMeanElementLength) +
            end.size());
  r += start;
  bool first = true;
  for (const T& e : vec) {
    if (first) {
      first = false;
    } else {
      r += sep;
    }
    detail::VectorToStringFormat(r, e);
  }
  r += end;
  return r;
}

}  // namespace DXU_NAMESPACE

#endif  // DXU_CONVERSION_H_INCLUDE
#ifdef DXU_CONVERSION_IMPLEMENTATION

namespace DXU_NAMESPACE {

namespace {
// Should set front to begging position.
// Returns last not of blank chars.
static size_t StringTrim(std::string& str, size_t& front) {
  static char kBlanks[] = " \t";  // std::isblank
  front = str.find_first_not_of(kBlanks, front);
  if (front == std::string::npos) {
    return std::string::npos;  // all blanks
  }
  size_t back = str.find_last_not_of(kBlanks);
  if (back + 1 < str.size()) {
    str[back + 1] = '\0';  // trim ending
  }
  return back;
}
}  // namespace

void TrimString(std::string& str) {
  size_t front = 0;
  size_t back = StringTrim(str, front);
  if (back == std::string::npos) {
    str.clear();  // all blanks
  } else {
    assert(front <= back);
    str = str.substr(front, back - front + 1);
  }
}

// any invalid format causes error
int StrToIntSafe(const char* str, int32_t& result, const int& base) {
  if (str == nullptr) return EADDRNOTAVAIL;
  char* end = nullptr;
  errno = 0;  // clear errno
  long num = std::strtol(str, &end, base);
  int err = errno;
  if (err) return err;
  if (end == nullptr || end == str) return EINVAL;
  if (*end != '\0') return EILSEQ;
  result = static_cast<int32_t>(num);
  return 0;
}

// any invalid format causes error
int StrToInt64Safe(const char* str, int64_t& result, const int& base) {
  if (str == nullptr) return EADDRNOTAVAIL;
  char* end = nullptr;
  errno = 0;  // clear errno
  long long num = std::strtoll(str, &end, base);
  int err = errno;
  if (err) return err;
  if (end == nullptr || end == str) return EINVAL;
  if (*end != '\0') return EILSEQ;
  result = static_cast<int>(num);
  return 0;
}

// @warn str trims ending in-place
int StrToIntWithTrim(std::string& str, int32_t& result, size_t front) {
  StringTrim(str, front);
  if (front == std::string::npos) {
    return EILSEQ;  // all blanks
  }
  return StrToIntSafe(str.c_str() + front, result);
}

// @warn str trims ending in-place
int StrToInt64WithTrim(std::string& str, int64_t& result, size_t front) {
  StringTrim(str, front);
  if (front == std::string::npos) {
    return EILSEQ;  // all blanks
  }
  return StrToInt64Safe(str.c_str() + front, result);
}

// parse "a:b", @warn str trims ending in-place
int StrToInt32PairWithTrim(std::string& str,
                           std::pair<int32_t, int32_t>& result, size_t front,
                           const char& sep) {
  auto back = StringTrim(str, front);
  if (front == std::string::npos) {
    return EILSEQ;  // all blanks
  }
  size_t pos = str.find(sep, front);
  if (pos == std::string::npos || pos == front || pos == back) {
    return EILSEQ;  // no separator or part
  }
  str[pos] = '\0';
  int err = StrToIntSafe(str.c_str() + front, result.first);
  if (err) return err;
  err = StrToIntSafe(str.c_str() + pos + 1, result.second);
  if (err) return err;
  return 0;
}

BytesUnit HumanReadableBytes(uint64_t bytes, std::string& result) {
  static const char units[] = {'\0', 'K', 'M', 'G', 'T', 'P'};
  if (bytes < 1024) {
    result = std::to_string(bytes);
    return SizeByte;
  }
  uint8_t unit = 0;
  double b = static_cast<double>(bytes);
  while (b >= 1024 && unit < SizePB) {
    b /= 1024;
    unit++;
  }
  char buf[8];  // max 16384PB
  int l = snprintf(buf, 8, "%.2lf%c", b, units[unit]);
  if (b >= 1000) {
    l -= 3;  // "00.00" -> "00"
  } else if (b >= 100) {
    l -= 1;  // "00.00" -> "00.0"
  }
  assert(l >= 1);
  buf[l - 1] = units[unit];
  buf[l] = '\0';
  result = buf;
  return static_cast<BytesUnit>(unit);
}

std::vector<int> StringToVectorInt(const std::string& str, int* err,
                                   const char& sep) {
  static char kBlanks[] = " \t";  // std::isblank
  std::vector<int> result;
  // accept trailing ','
  std::istringstream ss(str.back() == sep ? str.substr(0, str.size() - 1)
                                          : str);
  std::string num_str;
  int e = 0;
  while (std::getline(ss, num_str, sep)) {
    // printf("\"%s\" ", num_str.c_str());
    size_t front = num_str.find_first_not_of(kBlanks);
    if (front == std::string::npos) {
      e = EILSEQ;  // all blanks
      break;
    }
    int num = 0;
    int repeat = 1;
    int end_num = 0;
    size_t pos_repeat = num_str.find('*', front);
    size_t pos_range = num_str.find('-', front);
    if (pos_range == front) {
      pos_range = num_str.find('-', front + 1);  // skip negative sign
    }
    if (pos_repeat != std::string::npos && pos_range != std::string::npos) {
      e = EINVAL;  // can't have both '*' and '-'
      break;
    }
    // try parse repeat from "num*repeat"
    if (pos_repeat != std::string::npos) {
      e = StrToIntWithTrim(num_str, repeat, pos_repeat + 1);
      if (e) {
        break;
      }
      if (repeat <= 0) {
        e = EINVAL;
        break;
      }
      num_str = num_str.substr(front, pos_repeat - front);
      front = 0;
    }
    // try parse range from "num-end_num"
    if (pos_range != std::string::npos) {
      e = StrToIntWithTrim(num_str, end_num, pos_range + 1);
      if (e) {
        break;
      }
      num_str = num_str.substr(front, pos_range - front);
      front = 0;
    }
    // try parse "num"
    e = StrToIntWithTrim(num_str, num, front);
    if (e) {
      break;
    }
    if (pos_repeat != std::string::npos && repeat > 1) {
      result.reserve(result.size() + repeat);
      while (repeat--) {
        result.push_back(num);
      }
    } else if (pos_range != std::string::npos && end_num != num) {
      int step = num < end_num ? 1 : -1;
      end_num += step;  // should include end_num
      result.reserve(result.size() + std::abs(end_num - num));
      for (; num != end_num; num += step) {
        result.push_back(num);
      }
    } else {
      result.push_back(num);
    }
  }
  if (err) {
    *err = e;
  }
  if (e) {
    fprintf(stderr, "[%s] parse error %d for: \"%s\", invalid \"%s\"\n",
            __func__, e, str.c_str(), num_str.c_str());
    // not to clear result, return partial result
    // result.clear();
  }
  // result.shrink_to_fit();
  return result;
}

}  // namespace DXU_NAMESPACE

#endif  // DXU_CONVERSION_IMPLEMENTATION
