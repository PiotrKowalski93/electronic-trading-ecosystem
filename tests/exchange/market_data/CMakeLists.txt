add_executable(market_data_tests
    market_data_tests.cpp
)

target_link_libraries(market_data_tests
    PRIVATE
        gtest
        gtest_main
        libexchange
)

add_test(
    NAME market_data_tests
    COMMAND market_data_tests
)
