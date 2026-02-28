#ifndef DXU_SLICE_H_INCLUDE
#define DXU_SLICE_H_INCLUDE

#include <cassert>
#include <string>

#include "dxu/version.h"

#ifdef SUPPORT_STRING_VIEW
#include <string_view>
#endif  // SUPPORT_STRING_VIEW

#ifdef NDEBUG
// constexpr for release build
#define RelConstexpr constexpr
#else
// function with assert() or other debug checks
#define RelConstexpr
#endif

namespace DXU_NAMESPACE {

/**
 * Slice is a simple structure containing a pointer into some external storage
 * and its size. Slice is similar to std::string_view. User must ensure that the
 * slice is not used after the corresponding external storage has been
 * deallocated.
 */
class Slice {
 public:
  // Create an empty slice
  constexpr Slice() : data_(""), size_(0) {}

  // Create a slice that refers to d[0, n-1]
  constexpr Slice(const char* d, size_t n) : data_(d), size_(n) {}

  // [implicit] Create a slice that refers to the contents of "s"
  Slice(const std::string& s) : data_(s.data()), size_(s.size()) {}

  // [implicit] Create a slice that refers to s[0, strlen(s)-1]
  Slice(const char* s);

  // [implicit] Create an invalid slice that refers to nullptr
  constexpr Slice(std::nullptr_t) : data_(nullptr), size_(0) {}

  // Invalid slice that refers to nullptr
  static constexpr Slice Invalid() { return {nullptr, 0}; }

  // Return a pointer to the beginning of the referenced data
  constexpr const char* data() const { return data_; }

  // Return the length (in bytes) of the referenced data
  constexpr size_t size() const { return size_; }

  // Return true iff the length of the referenced data is zero
  constexpr bool empty() const { return size_ == 0; }

  // Return true iff not point to nullptr
  constexpr bool valid() const { return data_ != nullptr; }

  // Return the ith byte in the referenced data.
  // REQUIRES: n < size()
  RelConstexpr char operator[](size_t n) const {
    assert(valid() && n < size());
    return data_[n];
  }

  // Refer to an empty array
  constexpr void clear() {
    data_ = "";
    size_ = 0;
  }

  // Drop the first "n" bytes from this slice.
  // REQUIRES: n <= size()
  RelConstexpr void remove_prefix(size_t n) {
    assert(valid() && n <= size());
    data_ += n;
    size_ -= n;
  }

  // Drop the last "n" bytes from this slice.
  // REQUIRES: n <= size()
  RelConstexpr void remove_suffix(size_t n) {
    assert(valid() && n <= size());
    size_ -= n;
  }

  // Safely remove prefix.
  RelConstexpr void remove_prefix_s(size_t n) {
    if (n < size_) {
      remove_prefix(n);
    } else {
      data_ += size_;
      size_ = 0;
    }
  }

  // Safely remove suffix.
  RelConstexpr void remove_suffix_s(size_t n) {
    if (n < size_) {
      remove_suffix(n);
    } else {
      size_ = 0;
    }
  }

  // Return a string that contains the copy of the referenced data.
  std::string ToString() const { return {data_, size_}; }

  // Return a string that contains the copy of the referenced data.
  // If hex is true, returns a string of twice the length hex encoded (0-9A-F).
  std::string ToString(bool hex) const;

#ifdef SUPPORT_STRING_VIEW
  // [implicit] Create a slice that refers to the same contents as "sv"
  constexpr Slice(const std::string_view& sv)
      : data_(sv.data()), size_(sv.size()) {}

  // Return a string_view that references the same data as this slice.
  constexpr std::string_view ToStringView() const { return {data_, size_}; }
#endif  // SUPPORT_STRING_VIEW

  // Decodes the current slice interpreted as an hexadecimal string into result,
  // if successful returns true, if this isn't a valid hex string
  // (e.g not coming from Slice::ToString(true)) DecodeHex returns false.
  // This slice is expected to have an even number of 0-9A-F characters
  // also accepts lowercase (a-f)
  bool DecodeHex(std::string* result) const;

  // Three-way comparison. Returns value:
  //   <  0 iff "*this" <  "b",
  //   == 0 iff "*this" == "b",
  //   >  0 iff "*this" >  "b"
  int compare(const Slice& b) const;

  // Return true iff "x" is a prefix of "*this"
  bool starts_with(const Slice& x) const;

  // Return true iff "x" is a suffix of "*this"
  bool ends_with(const Slice& x) const;

  // Compare two slices and returns the first byte where they differ
  RelConstexpr size_t difference_offset(const Slice& b) const;

  // Return a new slice that removes leading and trailing whitespaces
  Slice trim() const { return trim_start().trim_end(); }

  // Remove trailing whitespaces
  Slice trim_end() const;

  // Remove leading whitespaces
  Slice trim_start() const;

  // Convert slice-like S to T
  template <class T, class S = Slice>
  class Converter {
   public:
    Converter(const S& s) : s_(s) {}
    operator T() const { return T(s_.data_, s_.size_); }
    const S& s_;
    static_assert(sizeof(*T::data_) == sizeof(*S::data_), "");
  };

  const char* data_;
  size_t size_;
};

bool operator==(const Slice& x, const Slice& y);

inline bool operator!=(const Slice& x, const Slice& y) { return !(x == y); }

}  // namespace DXU_NAMESPACE

#endif  // DXU_SLICE_H_INCLUDE
#ifdef DXU_SLICE_IMPLEMENTATION

#include <cctype>
#include <cstring>

namespace DXU_NAMESPACE {

Slice::Slice(const char* s) : data_(s), size_(s == nullptr ? 0 : strlen(s)) {}

bool Slice::starts_with(const Slice& x) const {
  return (size_ >= x.size_) && (0 == memcmp(data_, x.data_, x.size_));
}

bool Slice::ends_with(const Slice& x) const {
  return (size_ >= x.size_) &&
         (0 == memcmp(data_ + size_ - x.size_, x.data_, x.size_));
}

bool operator==(const Slice& x, const Slice& y) {
  return (x.size_ == y.size_) && (0 == memcmp(x.data_, y.data_, x.size_));
}

int Slice::compare(const Slice& b) const {
  assert(data_ != nullptr && b.data_ != nullptr);
  const size_t len = (size_ < b.size_) ? size_ : b.size_;
  int r = memcmp(data_, b.data_, len);
  if (r == 0) {
    if (size_ < b.size_) {
      r = -1;
    } else if (size_ > b.size_) {
      r = 1;
    }
  }
  return r;
}

RelConstexpr size_t Slice::difference_offset(const Slice& b) const {
  assert(data_ != nullptr && b.data_ != nullptr);
  size_t off = 0;
  const size_t len = (size_ < b.size_) ? size_ : b.size_;
  for (; off < len; ++off) {
    if (data_[off] != b.data_[off]) break;
  }
  return off;
}

namespace {
// v must be in [0, 15]
constexpr char toHex(char v) { return v <= 9 ? ('0' + v) : ('A' + v - 10); }

// c should be 0-9|A-F|a-f
constexpr int fromHex(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else {
    return -1;  // invalid hex digit
  }
}
}  // namespace

// Return a string that contains the copy of the referenced data.
std::string Slice::ToString(bool hex) const {
  if (!hex) return {data_, size_};
  if (data_ == nullptr) return "";
  std::string result;
  result.reserve(2 * size_);
  for (size_t i = 0; i < size_; ++i) {
    char c = data_[i];
    result.push_back(toHex(c >> 4));
    result.push_back(toHex(c & 0xf));
  }
  return result;
}

bool Slice::DecodeHex(std::string* result) const {
  // must be even number of hex digits
  if (size_ % 2 || data_ == nullptr || result == nullptr) return false;
  result->clear();
  result->reserve(size_ / 2);
  size_t i = 0;
  while (i < size_) {
    int h1 = fromHex(data_[i++]);
    if (h1 < 0) return false;
    int h2 = fromHex(data_[i++]);
    if (h2 < 0) return false;
    result->push_back(static_cast<char>((h1 << 4) | h2));
  }
  return true;
}

Slice Slice::trim_end() const {
  if (!valid() || empty()) return *this;
  const char* head = data_;
  const char* tail = data_ + size_ - 1;
  while (tail >= head && std::isspace(*tail)) {
    --tail;
  }
  return {head, static_cast<size_t>(tail - head + 1)};
}

Slice Slice::trim_start() const {
  if (!valid() || empty()) return *this;
  const char* head = data_;
  const char* end = data_ + size_;
  while (head < end && std::isspace(*head)) {
    ++head;
  }
  return {head, static_cast<size_t>(end - head)};
}

}  // namespace DXU_NAMESPACE

#endif  // DXU_SLICE_IMPLEMENTATION

#undef RelConstexpr
