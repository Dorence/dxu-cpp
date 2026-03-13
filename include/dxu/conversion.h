#ifndef DXU_CONVERSION_H_INCLUDE
#define DXU_CONVERSION_H_INCLUDE

#include <string>
#include <type_traits>
#include <vector>

#include "dxu/slice.h"

namespace DXU_NAMESPACE {

struct StrToIntOptions {
  int* err = nullptr;  // address to store error code
  int base = 0;        // 0 (auto), 2-36
  bool trim = true;    // trim before parse
  bool strict = true;  // no trailing content, etc.

  StrToIntOptions() noexcept = default;
  StrToIntOptions(int* _err) noexcept : err(_err) {}
  StrToIntOptions(int* _err, int _base, bool _trim, bool _strict) noexcept
      : err(_err), base(_base), trim(_trim), strict(_strict) {}
};

// wrapper for std::strtol, return 0 if error
int32_t StrToInt(const Slice& s, const StrToIntOptions& opts = {}) noexcept;

// wrapper for std::strtoll, return 0 if error
int64_t StrToInt64(const Slice& s, const StrToIntOptions& opts = {}) noexcept;

struct StrToIntPairOptions {
  int* err = nullptr;  // address to store error code
  int base = 0;        // 0 (auto), 2-36
  char sep = ':';      // seperator in "a:b"

  StrToIntPairOptions() noexcept = default;
  StrToIntPairOptions(int* _err) noexcept : err(_err) {}
  StrToIntPairOptions(int* _err, int _base, char _sep) noexcept
      : err(_err), base(_base), sep(_sep) {}
};

// parse "a:b", return {0, 0} if error
std::pair<int32_t, int32_t> StrToIntPair(
    const Slice& s, const StrToIntPairOptions& opts = {}) noexcept;

enum BytesUnit : uint8_t { SizeByte, SizeKB, SizeMB, SizeGB, SizeTB, SizePB };

// size <= 7 (16384PB)
BytesUnit HumanReadableBytes(uint64_t bytes, std::string& result);

struct StringToVectorIntOptions {
  int* err = nullptr;  // address to store error code
  int base = 0;        // 0 (auto), 2-36
  char sep = ',';      // seperator "1,2,3"
  char repeat = '*';   // repeat mark "1*3"
  char range = '-';    // range mark "0-9"

  StringToVectorIntOptions() noexcept = default;
  StringToVectorIntOptions(int* _err) noexcept : err(_err) {}
  StringToVectorIntOptions(int* _err, int _base, char _sep, char _repeat,
                           char _range) noexcept
      : err(_err), base(_base), sep(_sep), repeat(_repeat), range(_range) {}
};

/**
 * Features:
 * - Accepts spaces ' ' or '\t' and trailing sep ',': `"0  ,  0,0,"` -> [0, 0,
 * 0]
 * - '*' for repeat: `"1*3,8,-8,4*2"` -> [1, 1, 1, 8, -8, 4, 4]
 * - '-' for range:  `"1-3,0,2--1"` -> [1, 2, 3, 0, 2, 1, 0, -1]
 * - Custom seperator: `"1 2 3" (sep=' ')` -> [1, 2, 3]
 * Returns partial result if error.
 */
std::vector<int> StringToVectorInt(
    const Slice& s, const StringToVectorIntOptions& opts = {}) noexcept;

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
#if defined(DXU_CONVERSION_IMPLEMENTATION) || defined(__INTELLISENSE__)

#include <cassert>
#include <cerrno>

namespace DXU_NAMESPACE {

namespace {
inline int StrToIntPreCheck(Slice& s, bool need_trim) noexcept {
  if (!s.valid()) return EADDRNOTAVAIL;
  if (need_trim) s = s.trim();
  return s.empty() ? EINVAL : 0;
}

inline int StrToIntPostCheck(const char* start, const char* end, int err,
                             bool strict) noexcept {
  if (err) return err;
  if (end == nullptr || end == start) return EINVAL;
  if (strict && *end != '\0') return EILSEQ;
  return 0;
}

inline int StrToIntEat32(Slice& s, int32_t& num,
                         const StrToIntOptions& opts) noexcept {
  std::string str = s.ToString();  // copy to avoid non-zero-terminated
  char* end = nullptr;
  errno = 0;  // clear errno
  auto result = std::strtol(str.c_str(), &end, opts.base);
  int err = StrToIntPostCheck(str.c_str(), end, errno, opts.strict);
  if (err == 0) {
    num = static_cast<int32_t>(result);
    s.remove_prefix(end - str.c_str());
  }
  return err;
}

inline int StrToIntEat64(Slice& s, int64_t& num,
                         const StrToIntOptions& opts) noexcept {
  std::string str = s.ToString();  // copy to avoid non-zero-terminated
  char* end = nullptr;
  errno = 0;  // clear errno
  auto result = std::strtoll(str.c_str(), &end, opts.base);
  int err = StrToIntPostCheck(str.c_str(), end, errno, opts.strict);
  if (err == 0) {
    num = static_cast<int64_t>(result);
    s.remove_prefix(end - str.c_str());
  }
  return err;
}

class SliceSplitter {
 public:
  SliceSplitter(const Slice& s, const char sep) noexcept
      : s_(s.valid() ? s : Slice()), sep_(sep) {}

  Slice Next() noexcept {
    if (s_.empty()) return Slice::Invalid();  // done
    Slice res{s_};
    size_t pos = s_.find(sep_);
    if (pos == Slice::npos) {
      s_.clear();  // the last part
    } else {
      res = s_.substr(0, pos);    // pos in [0, size)
      s_.remove_prefix(pos + 1);  // advance to next part
    }
    return res;
  }

  void Shift(size_t offset) noexcept { s_.remove_prefix_s(offset); }

 private:
  Slice s_;
  const char sep_;
};
}  // namespace

int32_t StrToInt(const Slice& _s, const StrToIntOptions& opts) noexcept {
  Slice s{_s};
  int err = StrToIntPreCheck(s, opts.trim);
  int32_t num = 0;
  if (err == 0) err = StrToIntEat32(s, num, opts);
  if (opts.err != nullptr) *opts.err = err;
  return num;
}

int64_t StrToInt64(const Slice& _s, const StrToIntOptions& opts) noexcept {
  Slice s{_s};
  int err = StrToIntPreCheck(s, opts.trim);
  int64_t num = 0;
  if (err == 0) err = StrToIntEat64(s, num, opts);
  if (opts.err != nullptr) *opts.err = err;
  return num;
}

std::pair<int32_t, int32_t> StrToIntPair(
    const Slice& _s, const StrToIntPairOptions& opts) noexcept {
  int err = 0;
  int n1 = 0;
  int n2 = 0;
  if (!_s.valid()) err = EADDRNOTAVAIL;
  while (err == 0) {
    Slice s = _s.trim();
    if (s.empty()) {
      err = EILSEQ;  // all blanks
      break;
    }
    size_t pos = s.find(opts.sep);
    if (pos == Slice::npos || pos == 0 || pos == s.size() - 1) {
      err = EILSEQ;  // no separator or part
      break;
    }
    StrToIntOptions opts2{nullptr, opts.base, false, true};
    Slice s1 = s.substr(0, pos).trim_end();
    assert(!s1.empty());  // should not be empty
    err = StrToIntEat32(s1, n1, opts2);
    if (err != 0) break;
    Slice s2 = s.substr(pos + 1).trim_start();
    assert(!s2.empty());  // should not be empty
    err = StrToIntEat32(s2, n2, opts2);
    if (err != 0) n1 = 0;  // reset n1 if error
    break;
  }
  if (opts.err != nullptr) *opts.err = err;
  return {n1, n2};
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

std::vector<int> StringToVectorInt(
    const Slice& _s, const StringToVectorIntOptions& opts) noexcept {
  static const char kBlanks[] = " \t";  // std::isblank
  constexpr Slice kSliceBlanks{kBlanks, 2};
  std::vector<int> result;
  if (!_s.valid()) {
    if (opts.err != nullptr) *opts.err = EADDRNOTAVAIL;
    return result;
  }
  if (opts.range == opts.repeat || opts.sep == opts.repeat ||
      opts.range == opts.sep) {
    if (opts.err != nullptr) *opts.err = EINVAL;
    return result;
  }
  Slice s = _s.trim();
  if (s.empty()) {
    if (opts.err != nullptr) *opts.err = 0;  // empty array
    return result;
  }
  const bool sep_is_blank = kSliceBlanks.contains(opts.sep);
  if (!sep_is_blank && s[s.size() - 1] == opts.sep) {
    s.remove_suffix(1);  // remove trailing ','
  }
  int err = 0;
  StrToIntOptions opts2{nullptr, opts.base, false, true};
  SliceSplitter splitter{s, opts.sep};
  Slice str = splitter.Next();
  for (; str.valid(); str = splitter.Next()) {
    // printf("\"%s\" ", str.ToString().c_str());
    if (str.empty()) {
      if (sep_is_blank) {
        size_t start = str.data() - s.data() + 1;  // index in full string
        size_t n = start;
        while (n < s.size() && s[n] == opts.sep) {
          n++;
        }
        if (n > start) {
          splitter.Shift(n - start);  // skip consecutive blanks
        }
        continue;
      } else {
        err = EILSEQ;  // repeated sep "1,,2"
        break;
      }
    }
    Slice num_str = str.trim();
    if (num_str.empty()) {
      err = EILSEQ;  // empty element
      break;
    }
    int num = 0;
    int repeat = 1;
    int end_num = 0;
    size_t pos_repeat = num_str.find(opts.repeat);  // "a*b"
    size_t pos_range = num_str.find(opts.range);    // "a-b"
    if (opts.range == '-' && pos_range == 0) {
      pos_range = num_str.find(opts.range, 1);  // maybe negative sign
    }
    if (pos_repeat != Slice::npos && pos_range != Slice::npos) {
      err = EILSEQ;  // can't have both '*' and '-'
      break;
    }
    // try parse repeat from "num*repeat"
    else if (pos_repeat != Slice::npos) {
      Slice repeat_str = num_str.substr(pos_repeat + 1).trim_start();
      err = StrToIntEat32(repeat_str, repeat, opts2);
      if (err != 0) break;
      if (repeat <= 0) {
        err = EINVAL;
        break;
      }
      num_str = num_str.substr(0, pos_repeat).trim_end();
    }
    // try parse range from "num-end_num"
    else if (pos_range != Slice::npos) {
      Slice range_str = num_str.substr(pos_range + 1).trim_start();
      err = StrToIntEat32(range_str, end_num, opts2);
      if (err != 0) break;
      num_str = num_str.substr(0, pos_range).trim_end();
    }
    // try parse "num"
    err = StrToIntEat32(num_str, num, opts2);  // num_str is trimmed
    if (err != 0) break;
    if (pos_repeat != Slice::npos && repeat > 1) {
      result.reserve(result.size() + repeat);
      while (repeat--) {
        result.push_back(num);
      }
    } else if (pos_range != Slice::npos && end_num != num) {
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
  if (opts.err != nullptr) *opts.err = err;
  if (err) {
    fprintf(stderr, "[%s] parse error %d for: \"%s\", invalid \"%s\"\n",
            __func__, err, _s.ToString().c_str(), str.ToString().c_str());
    // dont clear result, return partial result
    // result.clear();
  }
  // result.shrink_to_fit();
  return result;
}

}  // namespace DXU_NAMESPACE

#endif  // DXU_CONVERSION_IMPLEMENTATION
