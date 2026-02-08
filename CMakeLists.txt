cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(tradingecosystem LANGUAGES CXX)

add_executable (tradingecosystem 
    exchange_main.cpp
)

add_subdirectory(common)
add_subdirectory(exchange)
add_subdirectory(tests)

target_link_libraries(tradingecosystem
    PRIVATE
        libcommon
        libexchange
)