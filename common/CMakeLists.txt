cmake_minimum_required (VERSION 3.8)

add_library(libcommon 
    INTERFACE)

target_include_directories(libcommon 
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR})