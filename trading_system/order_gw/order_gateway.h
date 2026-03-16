#pragma once

#include <errno.h>
#include <cstring>

#include "tcp_server.h"
#include "common_types.h"
#include "threads.h"
#include "client_request.h"
#include "client_response.h"

namespace TradingSystem
{
    // Client to Exchange
    class OrderGateway{
        public:
            OrderGateway(ClientId client_id, Exchange::ClientRequestLFQueue* outgoing_requests, Exchange::ClientResponseLFQueue* incoming_responses,
                std::string ip, const std::string iface, const int port);
            ~OrderGateway();

            auto run() noexcept -> void;

        private:
            const Common::ClientId client_id_;

            // Exchange address info
            std::string ip_;
            const std::string iface_;
            const int port_ = 0;

            Exchange::ClientRequestLFQueue* outgoing_requests_ = nullptr;
            Exchange::ClientResponseLFQueue* incoming_responses_ = nullptr;

            std::atomic<bool> is_running_;

            std::string time_str_;
            Logger logger_;

            size_t next_outgoing_seq_num_ = 1;
            size_t next_exp_response_seq_num_ = 1;
            Common::TCPSocket tcp_client_;

            auto start() -> void {
                is_running_ = true;
                
                // Connecting Gateway to the Exchange
                ASSERT(tcp_client_.connect(ip_, iface_, port_, false) >= 0, "Unable to connect to ip: " + ip_ +
                    " port: " + std::to_string(port_) + " iface: " + iface_ + " error: " + std::string(std::strerror(errno)));

                // Running processing on separate thread
                ASSERT(Common::createAndStartThread(-1, "Trading/OrderGateway", [this]() { run(); }) != nullptr, "Failed to start OrderGateway thread.");
            }

            auto stop() -> void {
                is_running_ = false;
            }

            auto recvCallback(TCPSocket* socket, Nanos rx_time) noexcept -> void;       
    };
}