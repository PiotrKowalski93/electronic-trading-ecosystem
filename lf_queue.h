#pragma once

#include <iostream>
#include <vector>
#include <atomic>

#include "macros.h"

namespace Common {
    template <typename T>
    class LFQueue final {
    public:
        LFQueue(size_t size) : store_(size, T()) {
        } 

        auto getNextToWriteTo() noexcept -> T* {
            return &store_[next_write_index_];
        }

        auto updateNextToWriteTo() noexcept {
            next_write_index_ = (next_write_index_ + 1) % store_.size();
            store_size_++;
        }

        // const noexcept -> const T * == const T* getNextToRead()
        // trailing return type <-- read more about it
        auto getNextToRead() const noexcept -> const T* {
            return (next_read_index_ == next_write_index_) ? nullptr : &store_[next_read_index_];
        }

        auto updateReadIndex() noexcept {
            next_read_index_ = (next_read_index_ + 1) % store_.size();
            ASSERT(store_size_ != 0, "Invalid store size: " + std::to_string(pthread_self()));
            store_size_--;
        }

        // const here means: This method does not change object state ("this")
        auto size() const noexcept {
            return store_size_.load();
        }

        LFQueue() = delete;
        LFQueue(const LFQueue&) = delete;
        LFQueue(const LFQueue&&) = delete;

        //TODO: Understand it
        LFQueue& operator=(const LFQueue&) = delete;
        LFQueue& operator=(const LFQueue&&) = delete;

        private:
            std::vector<T> store_;

            // size_t is used for indices and counters because it is the standard type
            // for sizes and memory indexing in C++; it is portable across architectures
            // (32/64-bit) and matches STL container sizes, avoiding truncation issues. 
            std::atomic<size_t> next_write_index_ = {0};
            std::atomic<size_t> next_read_index_ = {0};
            std::atomic<size_t> store_size_ = {0};
    };
}