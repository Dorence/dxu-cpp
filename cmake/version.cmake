## Check system type
if(CMAKE_SYSTEM_NAME MATCHES "Cygwin")
  add_definitions(-fno-builtin-memcmp -DCYGWIN)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  add_definitions(-DOS_MACOSX)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  add_definitions(-DOS_LINUX)
elseif(CMAKE_SYSTEM_NAME MATCHES "SunOS")
  add_definitions(-DOS_SOLARIS)
elseif(CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")
  add_definitions(-DOS_GNU_KFREEBSD)
elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  add_definitions(-DOS_FREEBSD)
elseif(CMAKE_SYSTEM_NAME MATCHES "NetBSD")
  add_definitions(-DOS_NETBSD)
elseif(CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
  add_definitions(-DOS_OPENBSD)
elseif(CMAKE_SYSTEM_NAME MATCHES "DragonFly")
  add_definitions(-DOS_DRAGONFLYBSD)
elseif(CMAKE_SYSTEM_NAME MATCHES "Android")
  add_definitions(-DOS_ANDROID)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
  add_definitions(-DWIN32 -DOS_WIN -D_MBCS -DWIN64 -DNOMINMAX)
  if(MINGW)
    add_definitions(-D_WIN32_WINNT=_WIN32_WINNT_VISTA)
  endif()
endif()

## Check system ablity
include(CheckCXXSourceCompiles)

CHECK_CXX_SOURCE_COMPILES("
#include <format>
int main() {
  std::string s = std::format(\"{}\", 42);
  return 0;
}
" SUPPORT_FORMAT)

CHECK_CXX_SOURCE_COMPILES("
#include <string_view>
int main() {
  std::string_view sv = \"bronya\";
  return 0;
}
" SUPPORT_STRING_VIEW)

CHECK_CXX_SOURCE_COMPILES("
#if defined(_MSC_VER) && !defined(__thread)
#define __thread __declspec(thread)
#endif
int main() {
  static __thread int tls;
  return 0;
}
" SUPPORT_THREAD_LOCAL)

## Check build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_BUILD_TYPE_DEBUG 1)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set(CMAKE_BUILD_TYPE_RELWITHDEBINFO 1)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_BUILD_TYPE_RELEASE 1)
endif()

## Print build directory
message("[dxu] Directories:")
message("        CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message("CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
message("CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message("        CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message("      PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

## Generate version.h
configure_file(include/dxu/version.h.in include/dxu/version.h ESCAPE_QUOTES)
