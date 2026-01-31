#pragma once

#include <chrono>
#include <ctime>
#include <string>

namespace Common {
    typedef int64_t Nanos;

    constexpr Nanos NANOS_TO_MICROS = 1000;
    constexpr Nanos MICROS_TO_MILLIS = 1000;
    constexpr Nanos MILLIS_TO_SECS = 1000;

    constexpr Nanos NANOS_TO_MILLIS = NANOS_TO_MICROS * MICROS_TO_MILLIS;
    constexpr Nanos NANOS_TO_SECS = NANOS_TO_MILLIS * MILLIS_TO_SECS;

    inline auto getCurrentNanos() noexcept {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    // This function is in classic write into buffer style
    // No allocations of string, because i pass already created and having it in same object that i log from, it is faster
    // to get it from the cache
    inline auto getCurrentTimeStr(std::string* time_str){
        // system_clock::to_time_t -> time_t is a C type, can be printed, seconds
        // std::chrono::system_clock::now() -> time_point, C++ time object, accurate point in time + clock type
        // Needs to be explicit convertion becouse precision is lost.
        const auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        time_str->assign(ctime(&time));

        // ctime() function adds '\n' at the end
        // we need to remove it so in logs, time will be in one line
        if(!time_str->empty()){
            time_str->at(time_str->length()-1) = '\0';
        }   

        return *time_str;     
    }
} 