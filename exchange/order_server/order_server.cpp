#pragma once

#include "order_server.h"

namespace Exchange {
    std::string order_server_log_file = "exchange_order_server.log";

    OrderServer::OrderServer(ClientRequestLFQueue* client_requests, ClientResponseLFQueue* client_responses, const std::string &iface, int port)
        : iface_(iface), port_(port), client_responses_(client_responses), logger(order_server_log_file), tcp_server_(logger) 
    { // Add init for fifo seq
        clientId_outgoing_seqNum_.fill(1);
        clientId_incoming_seqNum_.fill(1);
        clientId_socket_.fill(nullptr);

        // Lambda
        tcp_server_.recv_callback_ = [this](auto socket, auto rx_time){
            //recvCallback(socket,rx_time);
        };

        tcp_server_.recv_finished_callback_ = [this](){
            //recvFinishedCallback();
        };
    }

    auto OrderServer::start() -> void{
        is_running_ = true;
        tcp_server_.listen(iface_, port_);

        ASSERT(Common::createAndStartThread(-1, "Exchange/OrderServer", [this]() { run(); }) != nullptr, "Failed to start OrderServer thread.");
    }

    auto OrderServer::stop() -> void{
        is_running_ = false;
    }

    OrderServer::~OrderServer(){
        stop();

        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(1s);
    }
}