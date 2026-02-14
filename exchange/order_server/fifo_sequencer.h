#pragma once

#include <algorithm>

#include "thread_utils.h"
#include "macros.h"
#include "logging.h"

#include "client_request.h"

// FIFO Sequencer is subcomponent in order gateway server responsible for making sure
// that client requests are processed in the order of their arrival time
namespace Exchange{
    constexpr size_t ME_MAX_PENDING_REQUESTS = 1024;
    
    // Wrapper for client request with time
    struct RecvTimeClientRequest{
        Nanos recv_time = 0;
        MEClientRequest request;

        auto operator<(const RecvTimeClientRequest &rhs) const{
            return (recv_time < rhs.recv_time);
        }
    };
    
    class FIFOSequencer{
    public:
        FIFOSequencer(ClientRequestLFQueue* client_requests, Logger* logger) 
            : incoming_requests_(client_requests), logger_(logger){};

        ~FIFOSequencer(){};
        
        auto addClientRequest(Nanos rx_time, const MEClientRequest &client_request) -> void{
            if(pending_size_ >= pending_client_requests_.size()){
                FATAL("To many pending requests");
            }
            pending_client_requests_.at(pending_size_++) = std::move(RecvTimeClientRequest{rx_time, client_request});
        }

        auto sequenceAndPublish() -> void{
            if(UNLIKELY(!pending_size_)){
                return;
            }

            //Add log
            // We need to remove sorting - is it really needed?
            // Single threaded order gateway can give seq number when pooling request from sockets
            // In real time scenario NASDAQ gives seq number on NIC or FPGA sequencer far before our Matching Engine, and that gives fairness
            // Add notes about „ingress ordering” and „execution ordering”
            //TODO: Remove sort, rely on event loop order
            std::sort(pending_client_requests_.begin(), pending_client_requests_.begin() + pending_size_);

            for(size_t i = 0; i< pending_size_; ++i){
                const auto &client_request = pending_client_requests_.at(i);

                auto next_write = incoming_requests_->getNextToWriteTo();
                *next_write = std::move(client_request.request);
                incoming_requests_->updateNextToWriteTo();
            }
            pending_size_ = 0;
        }

        /// Deleted default, copy & move constructors and assignment-operators.
        FIFOSequencer() = delete;
        FIFOSequencer(const FIFOSequencer &) = delete;
        FIFOSequencer(const FIFOSequencer &&) = delete;
        FIFOSequencer &operator=(const FIFOSequencer &) = delete;
        FIFOSequencer &operator=(const FIFOSequencer &&) = delete;

    private:
        ClientRequestLFQueue* incoming_requests_ = nullptr;

        std::string time_str_;
        Logger* logger_;

        //TODO: Use mempool for RecvTimeClientRequest to avoid allocations
        std::array<RecvTimeClientRequest, ME_MAX_PENDING_REQUESTS> pending_client_requests_;
        size_t pending_size_ = 0;
    };
}