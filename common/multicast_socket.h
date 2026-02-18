#pragma once

#include <functional>
#include <socket_utils.h>
#include "logging.h"

namespace Common {
    constexpr size_t MulticastBufferSize = 64 * 1024 * 1024;
    
    struct MulticastSocket{

        MulticastSocket(Logger &logger): logger_(logger) {
            outbound_data_.resize(MulticastBufferSize);
            inbound_data_.resize(MulticastBufferSize);
        }

        // Initialize multicast socket, returns fd
        auto init(const std::string &ip, const std::string &iface, int port, bool is_listening) -> int;

        // Join to multicast group
        auto join(const std::string &ip)->bool;

        // Leave multicast group
        auto leave(const std::string &ip, int port) ->void;

        // publish outgoing data and read incoming
        auto sendAndRecv() noexcept -> void;

        // Copy data to send buffers
        auto send(const void* data, size_t len) noexcept -> void;

        int socket_fd_ = -1;

        std::vector<char> outbound_data_;
        size_t next_send_valid_index_ = 0;

        std::vector<char> inbound_data_;
        size_t next_rcv_valid_index_ = 0;

        // Function wrapper for the method to call when data is read.
        //- templating instead std::function
        //- function pointer
        //- intrusive callback
        std::function<void(MulticastSocket* s)> recv_callback_ = nullptr; //?????

        std::string time_str_;
        Logger &logger_;
    };
}