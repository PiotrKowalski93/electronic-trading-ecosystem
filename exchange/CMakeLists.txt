cmake_minimum_required (VERSION 3.8)

add_library(libexchange STATIC 
    matcher/matching_engine.cpp
    matcher/me_order.cpp
)

target_include_directories(libexchange
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(libexchange 
    PUBLIC 
        cxx_std_20)

target_link_libraries(libexchange
    PUBLIC 
        libcommon
)