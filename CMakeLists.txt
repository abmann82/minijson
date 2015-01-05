project(minijson)
cmake_minimum_required(VERSION 2.8)

# main library consists of minijson.h and minijson.cpp only.
# simply copy these files into your project to use it.
add_library(minijson STATIC src/minijson.cpp)

include_directories(${CMAKE_SOURCE_DIR}/src)

# optional tool: syntax validation of a file using minijson
add_executable(minijsonvalidate tools/minijsonvalidatemain.cpp)
target_link_libraries(minijsonvalidate minijson)

# optional minijson unittests, requires google test
# to build, download and extract google test (version 1.7.0 is known to work) and re-run cmake.
if (EXISTS "${CMAKE_SOURCE_DIR}/gtest/src/gtest-all.cc")
  message(STATUS "gtest/src/gtest-all.cc found, building unit tests.")
  find_package(Threads REQUIRED)
  include_directories(
    SYSTEM
    ${CMAKE_SOURCE_DIR}/gtest
    ${CMAKE_SOURCE_DIR}/gtest/include
  )
  add_executable(
    minijsontests
    tests/main.cpp
    tests/minijsontests.cpp
    gtest/src/gtest-all.cc
  )
  target_link_libraries(minijsontests minijson ${CMAKE_THREAD_LIBS_INIT})
else()
  message(STATUS "gtest/src/gtest-all.cc not found, omitting unit tests.")
endif ()

