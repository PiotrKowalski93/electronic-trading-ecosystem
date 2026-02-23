#pragma once

#include "thread_utils.h"
#include "macros.h"
#include "logging.h"
#include "tcp_server.h"

#include "client_request.h"
#include "client_response.h"
#include "fifo_sequencer.h"

namespace Exchange {
    class OrderServer{
    public:
        OrderServer(ClientRequestLFQueue *client_requests, ClientResponseLFQueue *client_responses, const std::string &iface, int port);
        ~OrderServer();

        /// Start and stop the order server main thread.
        auto start() -> void;
        auto run() noexcept -> void;
        auto stop() -> void;

        /// Deleted default, copy & move constructors and assignment-operators.
        OrderServer() = delete;
        OrderServer(const OrderServer &) = delete;
        OrderServer(const OrderServer &&) = delete;
        OrderServer &operator=(const OrderServer &) = delete;
        OrderServer &operator=(const OrderServer &&) = delete;

    private:
        const std::string iface_;
        const int port_ = 0;

        volatile bool is_running_ = false;

        std::string time_str_;
        Common::Logger logger_;

        /// Lock free queue of outgoing client responses to be sent out to connected clients.
        ClientResponseLFQueue* client_responses_ = nullptr;

        /// Hash map from ClientId -> TCP socket / client connection.
        std::array<Common::TCPSocket*, ME_MAX_NUM_CLIENTS> clientId_socket_;

        /// Hash map from ClientId -> the next sequence number to be sent on outgoing client responses.
        std::array<size_t, ME_MAX_NUM_CLIENTS> clientId_outgoing_seqNum_;
        /// Hash map from ClientId -> the next sequence number expected on incoming client requests.
        std::array<size_t, ME_MAX_NUM_CLIENTS> clientId_incoming_seqNum_;

        /// TCP server instance listening for new client connections.
        Common::TCPServer tcp_server_;

        /// FIFO sequencer responsible for making sure incoming client requests are processed in the order in which they were received.
        FIFOSequencer fifo_sequencer;

        auto recvCallback(TCPSocket* socket, Nanos rx_time) noexcept -> void;
        auto recvFinishedCallback() noexcept -> void;
    };
}