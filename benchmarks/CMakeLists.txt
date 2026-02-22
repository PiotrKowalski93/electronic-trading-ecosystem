cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(benchmarks LANGUAGES CXX)

add_executable (benchmarks 
    main.cpp
)
target_link_libraries(benchmarks
    PRIVATE
        libcommon
        libexchange
)