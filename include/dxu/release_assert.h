/**
 * @brief release_assert() and release_assert_msg()
 * Assertion in both debug and release mode, ignore NDEBUG.
 */
#ifndef DXU_RELEASE_ASSERT_H_INCLUDE
#define DXU_RELEASE_ASSERT_H_INCLUDE

#include "dxu/port/format.h"
#include "dxu/port/likely.h"
#include "dxu/version.h"

namespace DXU_NAMESPACE {
void OnAssertion(const char* assertion, const char* file, unsigned int line,
                 const char* function);

// file, line, function, assertion, ...
void OnAssertionMessage(const char* format, ...) DXU_PRINTF_ATTR(1, 2);
}  // namespace DXU_NAMESPACE

#if defined(__clang__) || __GNUC_PREREQ(2, 6)
#define __ASSERT_FUNCTION __extension__ __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define __ASSERT_FUNCTION __FUNCTION__
#else
#define __ASSERT_FUNCTION __func__
#endif  // __ASSERT_FUNCTION

#ifdef __FILE_NAME__
#define __ASSERT_FILE __FILE_NAME__
#else
// @todo(xun.deng): support short file name
#define __ASSERT_FILE __FILE__
#endif  // __ASSERT_FILE

#ifdef __GLIBC__
extern char* program_invocation_short_name;  // from errno.h
#define DXU_ON_ASSERTION_MESSAGE(fmt, expr, ...)              \
  ::DXU_NAMESPACE::OnAssertionMessage(                        \
      "%s: %s:%d: %s: Assertion `%s' failed. " fmt "\n",      \
      program_invocation_short_name, __ASSERT_FILE, __LINE__, \
      __ASSERT_FUNCTION, expr, ##__VA_ARGS__)
#else
#define DXU_ON_ASSERTION_MESSAGE(fmt, expr, ...)                              \
  ::DXU_NAMESPACE::OnAssertionMessage(                                        \
      "%s:%d: %s: Assertion `%s' failed. " fmt "\n", __ASSERT_FILE, __LINE__, \
      __ASSERT_FUNCTION, expr, ##__VA_ARGS__)
#endif  // DXU_ON_ASSERTION_MESSAGE

#define release_assert(expr)                                          \
  (UNLIKELY(!static_cast<bool>(expr))                                 \
       ? ::DXU_NAMESPACE::OnAssertion(#expr, __ASSERT_FILE, __LINE__, \
                                      __ASSERT_FUNCTION)              \
       : void(0))

#define release_assert_msg(expr, fmt, ...)                   \
  (UNLIKELY(!static_cast<bool>(expr))                        \
       ? DXU_ON_ASSERTION_MESSAGE(fmt, #expr, ##__VA_ARGS__) \
       : void(0))

#endif  // DXU_RELEASE_ASSERT_H_INCLUDE
#if defined(DXU_RELEASE_ASSERT_IMPLEMENTATION) || defined(__INTELLISENSE__)

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace DXU_NAMESPACE {
void OnAssertion(const char* assertion, const char* file, unsigned int line,
                 const char* function) {
#ifdef _GNU_SOURCE
  fprintf(stderr, "%s: %s:%d: %s: Assertion `%s' failed.\n",
          program_invocation_short_name, file, line, function, assertion);
#else
  fprintf(stderr, "%s:%d: %s: Assertion `%s' failed.\n", file, line, function,
          assertion);
#endif
  std::abort();
}

void OnAssertionMessage(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  std::abort();
}
}  // namespace DXU_NAMESPACE

#endif  // DXU_RELEASE_ASSERT_IMPLEMENTATION
