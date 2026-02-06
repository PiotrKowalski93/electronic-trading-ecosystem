cmake_minimum_required(VERSION 3.15)

include(FetchContent)

# GoogleTest
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

# Tests
add_subdirectory(common)
add_subdirectory(exchange)