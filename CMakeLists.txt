cmake_minimum_required (VERSION 3.8)

project(tradingecosystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(common)

list(APPEND LIBS libcommon)
list(APPEND LIBS libexchange)

add_executable (tradingecosystem "program.cpp" "constraints.h")

set_property(TARGET tradingecosystem PROPERTY CXX_STANDARD 17)