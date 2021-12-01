cmake_minimum_required(VERSION 3.20)
project(memsim)

set(CMAKE_CXX_STANDARD 14)

add_executable(memsim main.cpp memsim.cpp memsim.h)
