# Config catch2
include(ExternalProject)

find_package(Catch2 3 QUIET CONFIG)

if(NOT Catch2_FOUND)
  set(CATCH2_DIR ${CMAKE_BINARY_DIR}/third-party/catch2-project)
  ExternalProject_Add(
    catch2_project
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.13.0
    GIT_SHALLOW ON
    GIT_CONFIG ${EXTPROJ_GIT_CONFIG}
    PREFIX ${CATCH2_DIR}
    CMAKE_ARGS
      -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
      -DCMAKE_BUILD_TYPE=Release
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
      -DCATCH_INSTALL_DOCS=OFF
      -DCATCH_INSTALL_EXTRAS=ON
      -DCATCH_BUILD_TESTING=OFF
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_FLAGS:STRING=
      -DCMAKE_C_FLAGS:STRING=
      -DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}
    EXCLUDE_FROM_ALL ON # build on demand
    UPDATE_COMMAND ""   # disable rebuild
  )
  # list(APPEND CMAKE_PREFIX_PATH ${CATCH2_DIR}/lib/cmake)
  # find_package(Catch2 3 QUIET CONFIG)
  # if(NOT Catch2_FOUND)
  #   message(FATAL_ERROR "[Catch2] Failed to build Catch2")
  # endif()
  # message(STATUS "[Catch2] dir: ${Catch2_DIR}")

  add_library(Catch2 STATIC IMPORTED)
  add_dependencies(Catch2 catch2_project)
  set(CATCH2_LIBRARY ${CATCH2_DIR}/lib/libCatch2.a)
  set(CATCH2_LIB_MAIN ${CATCH2_DIR}/lib/libCatch2Main.a)
  # find_library(CATCH2_LIB_MAIN Catch2Main PATHS ${FOO_INSTALL_DIR}/lib)
  set(CATCH2_INCLUDE_DIRS ${CATCH2_DIR}/include)

  mark_as_advanced(CATCH2_LIBRARY CATCH2_INCLUDE_DIRS)
  message("[catch2]")
  message("lib Catch2: ${CATCH2_LIBRARY}")
  message("  lib Main: ${CATCH2_LIB_MAIN}")
  message("   include: ${CATCH2_INCLUDE_DIRS}")

  file(MAKE_DIRECTORY ${CATCH2_INCLUDE_DIRS})
  set_target_properties(Catch2 PROPERTIES
    IMPORTED_LOCATION "${CATCH2_LIB_MAIN}"
    INTERFACE_INCLUDE_DIRECTORIES "${CATCH2_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${CATCH2_LIBRARY}"
    POSITION_INDEPENDENT_CODE ON
  )
  # target_link_libraries(Catch2 INTERFACE ${CATCH2_LIBRARY})
else()
  message(STATUS "[catch2] reuse existing: ${Catch2_DIR}")
endif()
