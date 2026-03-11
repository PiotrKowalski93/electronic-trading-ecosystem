cmake_minimum_required (VERSION 3.8)

add_library(libtradingsystem STATIC 
    market_data/market_data_consumer.cpp
    strategy/market_order.cpp
    strategy/market_order_book.cpp
)

target_include_directories(libtradingsystem
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/market_data
        ${CMAKE_CURRENT_SOURCE_DIR}/strategy
)

target_compile_features(libtradingsystem 
    PUBLIC 
        cxx_std_20)

target_link_libraries(libtradingsystem
    PUBLIC 
        libcommon
        libexchange
)