#pragma once

#include <iostream>
#include <atomic>
#include <thread>
#include <unistd.h>

#include <sys/syscall.h>

inline auto setThreadCore(int core_id){
    // The cpu_set_t data structure represents a set of CPUs
    cpu_set_t cpuset;

    CPU_ZERO(&cpuset); // Clears set, so that it contains no CPUs.
    CPU_SET(core_id, &cpuset); //  Add CPU cpu to set.

    // pthread_setaffinity_np sets affinity, false if fails
    return (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) == 0);
}

// T &&func, A &&... args - forwarding reference
template<typename T, typename... A>
inline auto createAndStartThread(int core_id, const std::string &name, T &&func, A &&... args) noexcept{
    std::atomic<bool> running(false);
    std::atomic<bool> failed(false);

    auto thread_body = [&] {
        if(core_id >= 0 && !setThreadCore(core_id)){
            std::cerr << "Failed to set core affinity for " << name << " " << pthread_self() << " to " << core_id << std::endl;
            failed = true;
            return;
        }
        std::cerr << "Set core affinity for " << name << " " << pthread_self() << " to " << core_id << std::endl;
        
        running = true;
        std::forward<T>(func)(std::forward<A>(args)...);
    };

    auto t = new std::thread(thread_body);

    while(!running && !failed){
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(1s);
    }

    if (failed){
        t->join();
        delete t;
        t = nullptr;
    }

    return t;
}