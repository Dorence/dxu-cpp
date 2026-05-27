/**
 * Define printf-like format check macros.
 *
 * @author Dorence Deng <dorencedeng@gmail.com>
 */
#pragma once

#ifndef __GNUC_PREREQ
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define __GNUC_PREREQ(maj, min) \
  ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
#define __GNUC_PREREQ(maj, min) 0
#endif
#endif  // __GNUC_PREREQ

#if defined(__GNUC__) || defined(__clang__)
#define DXU_PRINTF_ATTR(fmt, arg) \
  __attribute__((__format__(__printf__, fmt, arg)))
#else
#define DXU_PRINTF_ATTR(fmt, arg)
#endif  // DXU_PRINTF_ATTR
