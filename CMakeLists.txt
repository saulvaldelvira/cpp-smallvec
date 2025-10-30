cmake_minimum_required(VERSION 3.5)
project(smallvec)

set(CMAKE_CXX_STANDARD 23)

add_executable(example main.cpp)
add_library(smallvec INTERFACE smallvec.hpp)