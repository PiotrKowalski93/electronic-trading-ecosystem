cmake_minimum_required (VERSION 3.8)

add_library(libexchange STATIC 
    matcher/matching_engine.cpp
    matcher/me_order.cpp
    matcher/me_order_book.cpp
)

target_include_directories(libexchange
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/market_data
        ${CMAKE_CURRENT_SOURCE_DIR}/order_server
        ${CMAKE_CURRENT_SOURCE_DIR}/matcher
)

target_compile_features(libexchange 
    PUBLIC 
        cxx_std_20)

target_link_libraries(libexchange
    PUBLIC 
        libcommon
)