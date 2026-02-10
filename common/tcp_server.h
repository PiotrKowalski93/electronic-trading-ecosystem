#pragma once

#include "tcp_socket.h"

namespace Common {
    struct TCPServer {
    private:
        auto defaultRecvCallback(TCPSocket* socket, Nanos rx_time) noexcept {
            logger_.log("%:% %() % TCPServer::defaultRecvCallback() socket:% len:% rx:%\n",
                __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), socket->fd_, socket->next_rcv_valid_index_, rx_time);
        }

        auto defaultRecvFinishedCallback() noexcept {
            logger_.log("%:% %() % TCPServer::defaultRecvFinishedCallback()\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));
        };

    public:
        int efd_ = -1; // Handler for kernel events
        TCPSocket listener_socket_;

        epoll_event events_[1024];

        std::vector<TCPSocket*> sockets_;
        std::vector<TCPSocket*> recv_sockets_;
        std::vector<TCPSocket*> send_sockets_;
        std::vector<TCPSocket*> disconnected_sockets_;

        std::function<void(TCPSocket* s, Nanos rx_time)> recv_callback_;
        std::function<void()> recv_finished_callback_;

        std::string time_str_;

        // Why not Logger*?
        // & - says that it is required, it must be present
        Logger &logger_;

        explicit TCPServer(Logger& logger) : listener_socket_(logger), logger_(logger){
            recv_callback_ = [this](auto socket, auto rx_time){
                defaultRecvCallback(socket, rx_time);
            };

            recv_finished_callback_ = [this]() {
                defaultRecvFinishedCallback();
            };
        }

        auto destroy();
        auto listen(const std::string &iface, int port) -> void;
        auto addToEpollList(TCPSocket *socket) noexcept -> bool;
        auto removeFromEpollList(TCPSocket *socket) noexcept -> bool;
        auto deleteSocket(TCPSocket *socket) noexcept -> void;
        auto poll() noexcept -> void;
        auto sendAndRecv() noexcept -> void;

        TCPServer() = delete;
        TCPServer(const TCPServer&) = delete;
        TCPServer(const TCPServer&&) = delete;
        TCPServer& operator=(const TCPServer&) = delete;
        TCPServer& operator=(const TCPServer&&) = delete;
    };
}