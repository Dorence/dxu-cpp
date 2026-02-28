cmake_minimum_required(VERSION 3.20)
project(dxu-cpp)

##---- Project Options ----##
set(DXU_NAMESPACE "dxu" CACHE STRING "Namespace for this project")

##----- Compile Options -----##
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic)
add_compile_options(-pthread)

if(CMAKE_BUILD_TYPE STREQUAL "" OR CMAKE_BUILD_TYPE STREQUAL "RD")
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()
message("[dxu] Build type: ${CMAKE_BUILD_TYPE}")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.cmake)

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
aux_source_directory(test TEST_SRCS)
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
endforeach()

add_custom_command(TARGET tests POST_BUILD
  COMMENT "Built all tests."
  COMMAND ${CMAKE_COMMAND} -E true
)
