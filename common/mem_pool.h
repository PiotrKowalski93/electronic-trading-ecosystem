#pragma once

#include "macros.h"
#include <vector>
#include <string>
#include <cstdint>

namespace Common {
    template <typename T>
    class MemPool final {
        public:
            explicit MemPool(std::size_t num_elems) : store_(num_elems, {T(), true}) // Vector prealocation
            {
                // Check if adress of T is adress of the whole block of ObjectBlock
                // possible optimalizations 
                ASSERT(reinterpret_cast<const ObjectBlock*>(&(store_[0].object_)) == &(store_[0]), "T object should be first member of ObjectBlock.");
            }

            template<typename... Args>
            T* allocate(Args... args) noexcept {
                auto obj_block = &(store_[next_free_index_]);

                ASSERT(obj_block->is_free_, "Expected free ObjectBlock at index:" + std::to_string(next_free_index_));

                T* block = &(obj_block->object_);

                // Placement new
                // new(address) Type(constructor params);
                block = new(block) T(args...);
                obj_block->is_free_ = false;
                
                updateNextFreeIndex();

                return block;
            }

            auto deallocate(const T *elem) noexcept {
                // std::ptrdiff_t 
                const auto elem_index = (reinterpret_cast<const ObjectBlock *>(elem) - &store_[0]);

                // Index is out of this pool indexes
                ASSERT(elem_index >= 0 && static_cast<size_t>(elem_index) < store_.size(), "Element being deallocated does not belong to this Memory pool.");

                // Already free 
                ASSERT(!store_[elem_index].is_free_, "Expected in-use ObjectBlock at index:" + std::to_string(elem_index));

                store_[elem_index].is_free_ = true;
            }

            MemPool() = delete;
            MemPool(const MemPool&) = delete;
            MemPool(const MemPool&&) = delete;

            MemPool& operator= (const MemPool&) = delete;
            MemPool& operator= (const MemPool&&) = delete;

        private:
            auto updateNextFreeIndex() noexcept {
                const auto init_next_free_index_ = next_free_index_;

                while(!store_[next_free_index_].is_free_){
                    ++next_free_index_;

                    // We end up at the end of vector
                    // back to beggining
                    if(UNLIKELY(next_free_index_ == store_.size())) {
                        next_free_index_ == 0;
                    }

                    // We looped over vector and there is no space
                    if(UNLIKELY(init_next_free_index_ == next_free_index_)){
                        // WHY ASSERT???
                        ASSERT(init_next_free_index_ != next_free_index_, "Memory Pool out of space");
                    }
                }
            }

            struct ObjectBlock {
                T object_;
                bool is_free_ = true;
            };
        
        // We could've chosen to use a std::array that would allocate the memory on the stack instead of the heap.
        // We would have to measure to see which one yields better performance.
        // TODO: Mesure Perf on array vs vector - when you will know how  :D
        std::vector<ObjectBlock> store_;
        size_t next_free_index_ = 0;
    };
}
