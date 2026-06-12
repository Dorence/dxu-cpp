/**
 * Define portable nodiscard macro.
 *
 * @author Dorence Deng <dorencedeng@gmail.com>
 */
#pragma once

#ifndef NODISCARD
/** [FEATURE] Uncomment to enable NODISCARD(msg), incompatible with NODISCARD */
// #define ENABLE_NODISCARD_MSG 1

#if defined(ENABLE_NODISCARD_MSG) && ENABLE_NODISCARD_MSG
/** Portable [[nodiscard(msg)]] */
#if __cplusplus >= 202002L
#define NODISCARD(msg) [[nodiscard(msg)]]
#elif __cplusplus >= 201703L
#define NODISCARD(msg) [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
#define NODISCARD(msg) __attribute__((warn_unused_result))
#elif defined(_MSC_VER)
#include <sal.h>
#define NODISCARD(msg) _Check_return_
#else
#define NODISCARD(msg)
#endif  // define NODISCARD(msg)

#else
/** Portable [[nodiscard]] */
#if __cplusplus >= 201703L
#define NODISCARD [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
#define NODISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER)
#include <sal.h>
#define NODISCARD _Check_return_
#else
#define NODISCARD
#endif  // define NODISCARD

#endif  // ENABLE_NODISCARD_MSG
#endif  // NODISCARD
