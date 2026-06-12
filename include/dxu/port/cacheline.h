/**
 * Define CACHE_LINE_SIZE and padding macros.
 *
 * @author Dorence Deng <dorencedeng@gmail.com>
 */
#pragma once

#ifndef CACHE_LINE_SIZE
#if defined(__s390__)
#if defined(__GNUC__) && __GNUC__ < 7
#define CACHE_LINE_SIZE 64U
#else
#define CACHE_LINE_SIZE 256U
#endif
#elif defined(__powerpc__) || defined(__aarch64__)
#define CACHE_LINE_SIZE 128U
#else
#define CACHE_LINE_SIZE 64U
#endif
#endif  // CACHE_LINE_SIZE

static_assert((CACHE_LINE_SIZE & (CACHE_LINE_SIZE - 1)) == 0,
              "Cache line size must be power of 2");

#ifndef ALIGN_CL
// Make fields cache line aligned
#define ALIGN_CL alignas(CACHE_LINE_SIZE)
// Calculate padding bytes for cache line alignment
#define CALC_CL_PAD(sz) (CACHE_LINE_SIZE - (sz) % CACHE_LINE_SIZE)
// Padding field for cache line alignment
#define FIELD_CL_PAD(name, sz) char name[CALC_CL_PAD(sz)]
#endif
