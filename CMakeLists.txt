cmake_minimum_required(VERSION 3.2.0)
include(CheckCXXCompilerFlag)

project(aiss4)

set (CMAKE_VERBOSE_MAKEFILE ON)

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Debug|Release|RelWithDebInfo|MinSizeRel" FORCE)
endif()

check_cxx_compiler_flag (-Wall HAS_WALL)
if (HAS_WALL)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

check_cxx_compiler_flag (-funroll-loops HAS_LOOP_UNROLL)
if (HAS_LOOP_UNROLL)
    set (CMAKE_CXX_FLAGS "-funroll-loops ${CMAKE_CXX_FLAGS}")
endif()

check_cxx_compiler_flag (-xHost HAS_XHOST)
check_cxx_compiler_flag (-march=native HAS_MARCH_NATIVE)
if (HAS_XHOST)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -xHost")
elseif (HAS_MARCH_NATIVE)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

check_cxx_compiler_flag (-ipo HAS_IPO)
if (HAS_IPO)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ipo")
endif()

check_cxx_compiler_flag (-flto HAS_FLTO)
if (HAS_FLTO)
    set (CMAKE_CXX_FLAGS "-flto ${CMAKE_CXX_FLAGS}")
endif()

enable_testing()

file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

configure_file (${CMAKE_SOURCE_DIR}/tests/test3.cpp.in ${CMAKE_BINARY_DIR}/tests/test3.cpp)
configure_file (${CMAKE_SOURCE_DIR}/tests/test4.cpp.in ${CMAKE_BINARY_DIR}/tests/test4.cpp)
configure_file (${CMAKE_SOURCE_DIR}/tests/test5.cpp.in ${CMAKE_BINARY_DIR}/tests/test5.cpp)
configure_file (${CMAKE_SOURCE_DIR}/tests/test6.cpp.in ${CMAKE_BINARY_DIR}/tests/test6.cpp)

add_executable(test1 ${CMAKE_SOURCE_DIR}/tests/test1.cpp)
add_executable(test2 ${CMAKE_SOURCE_DIR}/tests/test2.cpp)
add_executable(test3 ${CMAKE_BINARY_DIR}/tests/test3.cpp)
add_executable(test4 ${CMAKE_BINARY_DIR}/tests/test4.cpp)
add_executable(test5 ${CMAKE_BINARY_DIR}/tests/test5.cpp)
add_executable(test6 ${CMAKE_BINARY_DIR}/tests/test6.cpp)

target_include_directories(test1 PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(test2 PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(test3 PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(test4 PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(test5 PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(test6 PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(banana          test1)
add_test(baabaabac       test2)
add_test(chr22.dna.512kB test3)
add_test(etext99.1MB     test4)
add_test(chr22.dna.full  test5)
add_test(etext99.full    test6)

