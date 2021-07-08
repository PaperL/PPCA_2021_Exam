cmake_minimum_required(VERSION 3.16)
project(1)

set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_EXE_LINKER_FLAGS "-Wl,--stack,500000000")

add_executable(code src/main.cpp src/compress.hpp)