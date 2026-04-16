#pragma once

#ifndef LIKELY
#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x) (__builtin_expect((x), 1))
#else
#define LIKELY(x) (x)
#endif  // defined(__GNUC__) && __GNUC__ >= 4
#endif  // LIKELY

#ifndef UNLIKELY
#if defined(__GNUC__) && __GNUC__ >= 4
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define UNLIKELY(x) (x)
#endif  // defined(__GNUC__) && __GNUC__ >= 4
#endif  // UNLIKELY
