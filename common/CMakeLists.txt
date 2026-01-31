cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "*.h")

include_directories(${PROJECT_SOURCE_DIR})

add_library(libcommon INTERFACE ${SOURCES})