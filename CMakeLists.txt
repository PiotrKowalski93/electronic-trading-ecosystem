cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(exchange LANGUAGES CXX)

add_executable (exchange 
    exchange_main.cpp
)

add_subdirectory(common)
add_subdirectory(exchange)
add_subdirectory(tests)
add_subdirectory(benchmarks)

target_link_libraries(exchange
    PRIVATE
        libcommon
        libexchange
)