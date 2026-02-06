add_executable(common_tests
    common_dummy_test.cpp
)

target_link_libraries(common_tests
    PRIVATE
        gtest
        gtest_main
)

add_test(
    NAME common_tests
    COMMAND common_tests
)
