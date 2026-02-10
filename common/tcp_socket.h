#pragma once

#include <functional>

#include "logging.h"
#include "socket_utils.h"

namespace Common {
    constexpr size_t TCPBufferSize = 64 * 1024 * 1024;

    //use vector<char> or unique_ptr<char[]> ?
    struct TCPSocket {
        int fd_ = -1;

        bool send_disconnected_ = false;
        char *send_buffer_ = nullptr;
        size_t next_send_valid_index_ = 0;

        bool rcv_disconnected = 0;
        char *rcv_buffer_ = nullptr;
        size_t next_rcv_valid_index_ = 0;

        // I have to give struct bcs some C stuff.
        // It will compile without it;
        struct sockaddr_in inInAddr;

        // Callback type object, function = universal function wrapper
        std::function<void(TCPSocket *s, Nanos rx_time)> recv_callback_;

        auto destory() noexcept -> void;
        auto connect(const std::string &ip, const std::string &iface, int port, bool is_listening) -> int;
        auto send(const void *data, size_t len) noexcept -> void;
        auto sendAndRecv() noexcept -> bool;

        std::string time_str_;
        Logger &logger_;

        auto defaultRcvCallback(TCPSocket *socket, Nanos rx_time) noexcept {
             logger_.log("TCPServer::defaultRecvCallback() socket:% len:% rx:%\n", socket->fd_, socket->next_rcv_valid_index_, rx_time);
        }

        explicit TCPSocket(Logger &logger) : logger_(logger) {
            send_buffer_ = new char[TCPBufferSize];
            rcv_buffer_ = new char[TCPBufferSize];

            recv_callback_ = [this](auto socket, auto rx_time){
                defaultRcvCallback(socket, rx_time);
            };
        }

        ~TCPSocket() {
            destory();
            delete[] send_buffer_;
            send_buffer_ = nullptr;

            delete[] rcv_buffer_;
            rcv_buffer_ = nullptr;
        }

        TCPSocket() = delete;
        TCPSocket(const TCPSocket&) = delete;
        TCPSocket(const TCPSocket&&) = delete;
        TCPSocket& operator=(const TCPSocket&) = delete;
        TCPSocket& operator=(const TCPSocket&&) = delete;
    };
}
