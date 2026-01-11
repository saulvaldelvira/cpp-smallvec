cmake_minimum_required(VERSION 3.10)
project(smallvec)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb")

add_executable(example main.cpp)
add_library(smallvec INTERFACE smallvec.hpp)
