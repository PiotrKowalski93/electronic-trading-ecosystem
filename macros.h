#pragma once

#include <iostream>

//One of the most used optimization techniques in the Linux kernel is " __builtin_expect". 
//When working with conditional code (if-else statements), we often know which branch is true and which is not. 
//If compiler knows this information in advance, it can generate most optimized code.

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
// in C++20 [[likely]] [[unlikely]]

inline auto ASSERT(bool cond, const std::string &msg) noexcept {
    if(UNLIKELY(!cond)){
        std::cerr << msg << std::endl;
        exit(EXIT_FAILURE);
    }
}

inline auto FATAL(const std::string &msg) noexcept {
    std::cerr << msg << std::endl;
    exit(EXIT_FAILURE);
}