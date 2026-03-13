#ifndef DXU_CHRONO_H_INCLUDE
#define DXU_CHRONO_H_INCLUDE

#include <cstdint>
#include <string>

#include "dxu/format.h"

namespace DXU_NAMESPACE::chrono {

// std::micro::den = 1e6
constexpr uint64_t kMicrosInSecond = 1000 * 1000;
// std::nano::den = 1e9
constexpr uint64_t kNanosInSecond = 1000 * 1000 * 1000;
// std::ratio_divide<std::nano, std::micro>::den = 1e3
constexpr uint64_t kNanosInMicro = 1000;

// microseconds since the UTC epoch.
uint64_t NowMicros();

// microseconds since the Steady Clock epoch.
uint64_t NowMicrosSteady();

// nanoseconds since the Steady Clock epoch.
uint64_t NowNanos();

// sleep for microseconds.
void SleepMicros(uint32_t micros);

// "YYYY/mm/dd HH:MM:SS"
std::string TimeToString(uint64_t secondsSince1970) noexcept;

inline std::string NowTimeString() {
  return TimeToString(NowMicros() / kMicrosInSecond);
}

namespace detail {
// Write "AA-BB-CC" to out, where a, b, c are in [0, 100) and sep is a char.
void WriteTripleDigit2(char* out, uint32_t a, uint32_t b, uint32_t c,
                       char sep) noexcept;
}  // namespace detail

}  // namespace DXU_NAMESPACE::chrono

#endif  // DXU_CHRONO_H_INCLUDE
#if defined(DXU_CHRONO_IMPLEMENTATION) || defined(__INTELLISENSE__)

#include <sys/time.h>

#include <cassert>
#include <chrono>
#include <thread>

#ifdef OS_MACOSX
#include <mach/clock.h>
#include <mach/mach.h>
#endif  // OS_MACOSX

namespace DXU_NAMESPACE::chrono {
namespace {
template <class Clock = std::chrono::system_clock,
          class Duration = std::chrono::nanoseconds>
inline int64_t StdClockNow() {
  return std::chrono::duration_cast<Duration>(Clock::now().time_since_epoch())
      .count();
}

// offset in ns between real and mono clocks
int64_t ClockOffset() {
  static int64_t offset = [] {
#if defined(OS_LINUX) || defined(OS_FREEBSD) || defined(OS_GNU_KFREEBSD) || \
    defined(OS_AIX)
    struct timespec real, mono;
    clock_gettime(CLOCK_REALTIME, &real);
    clock_gettime(CLOCK_MONOTONIC, &mono);
    int64_t real_ns = real.tv_sec * kNanosInSecond + real.tv_nsec;
    int64_t mono_ns = mono.tv_sec * kNanosInSecond + mono.tv_nsec;
#elif defined(OS_MACOSX)
    int64_t real_ns = 0, mono_ns = 0;  // NowNanos() uses CALENDAR_CLOCK
#else
    int64_t real_ns = StdClockNow<std::chrono::system_clock>();
    int64_t mono_ns = StdClockNow<std::chrono::steady_clock>();
#endif
    // fprintf(stderr, "real: %lld, mono: %lld, diff: %lld\n", real_ns, mono_ns,
    //         real_ns - mono_ns);
    return real_ns - mono_ns;
  }();
  return offset;
}

// Write "AA-BB-CC" to out, where a, b, c are in [0, 100) and sep is a char.
inline void Write3Digit2(char* out, uint32_t a, uint32_t b, uint32_t c,
                         char sep) noexcept {
  uint64_t digits = a | (b << 24) | (static_cast<uint64_t>(c) << 48);
  // convert (10m+n) to BCD (16m+n).
  digits += (((digits * 205) >> 11) & 0x000f00000f00000f) * 6;
  // Put low nibbles to high bytes and high nibbles to low bytes.
  digits = ((digits & 0x00f00000f00000f0) >> 4) |
           ((digits & 0x000f00000f00000f) << 8);
  // Add ASCII '0' to each digit byte and insert separators.
  const auto usep = static_cast<uint64_t>(sep);
  digits |= 0x3030003030003030 | (usep << 16) | (usep << 40);
  format::PutFixed64(out, digits);
}
}  // namespace

uint64_t NowMicros() {
#if defined(OS_WIN)
  return StdClockNow<std::chrono::system_clock, std::chrono::microseconds>();
#else
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return static_cast<uint64_t>(tv.tv_sec) * kMicrosInSecond + tv.tv_usec;
#endif
}

uint64_t NowMicrosSteady() {
#if defined(OS_LINUX) || defined(OS_FREEBSD) || defined(OS_GNU_KFREEBSD) || \
    defined(OS_AIX)
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);  // gettimeofday() is not monotonic
  return ts.tv_sec * kMicrosInSecond +
         (ts.tv_nsec + ClockOffset()) / kNanosInMicro;
#else
  return (NowNanos() + ClockOffset()) / kNanosInMicro;
#endif
}

uint64_t NowNanos() {
#if defined(OS_LINUX) || defined(OS_FREEBSD) || defined(OS_GNU_KFREEBSD) || \
    defined(OS_AIX)
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<uint64_t>(ts.tv_sec) * kNanosInSecond + ts.tv_nsec;
#elif defined(OS_SOLARIS)
  return gethrtime();
#elif defined(OS_MACOSX)
  clock_serv_t cclock;
  mach_timespec_t ts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &ts);
  mach_port_deallocate(mach_task_self(), cclock);
  return static_cast<uint64_t>(ts.tv_sec) * kNanosInSecond + ts.tv_nsec;
#else
  return StdClockNow<std::chrono::steady_clock>();
#endif
}

void SleepMicros(uint32_t micros) {
  std::this_thread::sleep_for(std::chrono::microseconds(micros));
}

std::string TimeToString(uint64_t secondsSince1970) noexcept {
  const time_t seconds = static_cast<time_t>(secondsSince1970);
  struct tm t;
  localtime_r(&seconds, &t);
  std::string str{"0000/00/00-00:00:00"};  // 19 chars
  char* const s = const_cast<char*>(str.data());
  // first 2 digits of "YYYY"
  format::WriteDigit2(s, (t.tm_year / 100 + 19) % 100);
  // "YY/mm/dd"
  Write3Digit2(s + 2, t.tm_year % 100, t.tm_mon + 1, t.tm_mday, '/');
  // "HH:MM:SS"
  Write3Digit2(s + 11, t.tm_hour, t.tm_min, t.tm_sec, ':');
  return str;
}

namespace detail {
void WriteTripleDigit2(char* out, uint32_t a, uint32_t b, uint32_t c,
                       char sep) noexcept {
  Write3Digit2(out, a % 100, b % 100, c % 100, sep);
}
}  // namespace detail

}  // namespace DXU_NAMESPACE::chrono

#endif  // DXU_CHRONO_IMPLEMENTATION
