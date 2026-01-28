cmake_minimum_required (VERSION 3.8)

project(sockets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable (sockets "program.cpp" "common.cpp")

set_property(TARGET sockets PROPERTY CXX_STANDARD 17)