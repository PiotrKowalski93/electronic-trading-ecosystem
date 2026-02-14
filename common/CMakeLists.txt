cmake_minimum_required (VERSION 3.8)

add_library(libcommon STATIC
    socket_utils.cpp
    tcp_server.cpp
    tcp_socket.cpp)

target_include_directories(libcommon 
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR})