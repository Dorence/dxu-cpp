cmake_minimum_required(VERSION 3.20)
project(dxu-cpp)

##---- Project Options ----##
set(DXU_NAMESPACE "dxu" CACHE STRING "Namespace for this project")
set(DXU_CONFIG_EXPORT_DIR "include" CACHE STRING
  "Directory to export dxu/version.h, relative to CMAKE_CURRENT_BINARY_DIR")

##----- Compile Options -----##
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
  add_compile_options(
    /W4          # rough analogue of -Wall -Wextra
    /permissive- # disable MSVC language extensions, enforce standard C++
  )
else()
  add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wcast-align             # cast increases required alignment
    -Wdouble-promotion       # implicit float-to-double promotion
    -Wformat=2               # strict printf/scanf format checks
    -Wimplicit-fallthrough   # missing break in switch
    -Wmisleading-indentation # indentation vs control flow mismatch
    -Wnon-virtual-dtor       # polymorphic class with non-virtual dtor
    -Wnull-dereference       # null pointer dereference
    -Wold-style-cast         # forbid C-style casts
    -Woverloaded-virtual     # derived method hides a virtual
    -Wshadow                 # shadowed identifier
    -Wsign-compare           # signed/unsigned comparison
    -Wundef                  # undefined macro in #if
    -Wunused                 # unused vars/params/values/functions
  )
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
      -Wduplicated-branches # if/else with identical bodies
      -Wduplicated-cond     # repeated condition in if/else-if chain
      -Wlogical-op          # suspicious logical operator usage
      -Wuseless-cast        # cast to the same type
    )
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
      -Wextra-semi # redundant semicolon after a definition
      -Wshadow-all # broader -Wshadow
    )
  endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "" OR CMAKE_BUILD_TYPE STREQUAL "RD")
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()
message("[dxu] Build type: ${CMAKE_BUILD_TYPE}")

if(MSVC)
  add_compile_options(
    /Zc:__cplusplus  # real __cplusplus
    /Zc:preprocessor # use the conformant token-based preprocessor
    /Zc:inline       # remove unreferenced inline functions/data
    /Zc:wchar_t      # treat wchar_t as built-in
    /Zc:forScope     # standard-conformant scope of for-loop variables
    /utf-8           # interpret source and execution charset as UTF-8
    /EHsc            # C++ exception, assume extern "C" noexcept
    /GR              # enable RTTI
    /nologo          # suppress startup banner
  )
else()
  add_compile_options(-pthread)

  if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fno-omit-frame-pointer)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-momit-leaf-frame-pointer" DXU_HAVE_OMIT_LEAF_FP)
    if(DXU_HAVE_OMIT_LEAF_FP)
      add_compile_options(-momit-leaf-frame-pointer)
    endif()
  endif()
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dxu-version.cmake)

##---- External Project Options ----##
set(GIT_PROXY "http://sys-proxy-rd-relay.byted.org:8118" CACHE STRING "Github/Gitlab proxy")
set(EXTPROJ_GIT_CONFIG "http.https://github.com.proxy=${GIT_PROXY} http.https://gitlab.com.proxy=${GIT_PROXY}"
  CACHE STRING "ExternalProject_Add GIT_CONFIG")

##----- Tests -----##
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/catch2.cmake)

add_library(dxu-cpp INTERFACE)
target_include_directories(dxu-cpp INTERFACE include ${CMAKE_CURRENT_BINARY_DIR}/include)

add_library(testlib STATIC test/test_common.cc)
target_link_libraries(testlib PUBLIC dxu-cpp Catch2)

add_custom_target(tests)
set(TEST_BIN)

aux_source_directory(test TEST_SRCS)
list(REMOVE_DUPLICATES TEST_SRCS)
foreach(TARGET_FILE ${TEST_SRCS})
  get_filename_component(TARGET ${TARGET_FILE} NAME_WE)
  if("${TARGET}" STREQUAL "test_common")
    continue()
  endif()
  add_executable(${TARGET} ${TARGET_FILE})
  target_link_libraries(${TARGET} PRIVATE testlib)
  set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test
  )
  add_dependencies(tests ${TARGET})
  list(APPEND TEST_BIN ${TARGET})
endforeach()
list(REMOVE_DUPLICATES TEST_BIN)
string(JOIN " " TEST_BIN ${TEST_BIN})

aux_source_directory(test/except TEST_EXCEPT_SRCS)
list(REMOVE_DUPLICATES TEST_EXCEPT_SRCS)
foreach(TARGET_FILE ${TEST_EXCEPT_SRCS})
  get_filename_component(TARGET ${TARGET_FILE} NAME_WE)
  add_executable(${TARGET} ${TARGET_FILE})
  target_link_libraries(${TARGET} PRIVATE testlib)
  set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/except
  )
  add_dependencies(tests ${TARGET})
endforeach()

message("TEST_SRCS: ${TEST_SRCS}")
message("TEST_EXCEPT_SRCS: ${TEST_EXCEPT_SRCS}")
message("TEST_BIN: ${TEST_BIN}")

# make tests
add_custom_command(TARGET tests POST_BUILD
  COMMENT "Built all tests."
  COMMAND ${CMAKE_COMMAND} -E true
)

# make run_tests
add_custom_target(run_tests DEPENDS tests)
add_custom_command(TARGET run_tests POST_BUILD
  COMMENT "Running all tests."
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test/runner.sh ARGS ${TEST_BIN}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
