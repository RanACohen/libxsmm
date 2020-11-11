cmake_minimum_required(VERSION 3.12)
project(libxsmm)


set(CMAKE_CXX_STANDARD 14)
set(LIBXSMM_ROOT ${CMAKE_CURRENT_LIST_DIR})

include_directories(${LIBXSMM_ROOT}/include)
include_directories(${LIBXSMM_ROOT}/src)

add_subdirectory(src)
add_subdirectory(samples)
