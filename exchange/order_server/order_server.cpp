#pragma once

#include "order_server.h"

namespace Exchange {
    std::string order_server_log_file = "exchange_order_server.log";

    auto OrderServer::recvCallback(TCPSocket* socket, Nanos rx_time) noexcept -> void{
        // Add log
        if(socket->next_rcv_valid_index_ >= sizeof(OMClientRequest)){
            size_t i = 0;
            for(; i + sizeof(OMClientRequest) <= socket->next_rcv_valid_index_; i += sizeof(OMClientRequest)) {
                auto request = reinterpret_cast<OMClientRequest*>(socket->rcv_buffer_ + i);
                // Add Log

                // Add new socket connection for CliendId
                if(UNLIKELY(clientId_socket_[request->me_client_request.clientId_] == nullptr)){
                    clientId_socket_[request->me_client_request.clientId_] = socket;
                }
                
                //For security reasons we skip message if it comes from different socket for the client we have registered
                //we do not throw on a hot-path
                if(clientId_socket_[request->me_client_request.clientId_] != socket){
                    //Add log
                    continue;
                }

                // Check seq number for the request
                auto& next_seq_num = clientId_outgoing_seqNum_[request->me_client_request.clientId_];
                if(request->seq_num != next_seq_num){
                    //Log
                    continue;
                }

                // increment seq num for this client
                ++next_seq_num;

                //fifo_sequencer.addClientRequest(...) ??
            }

            // Remove readed from buffer, use memmove()?
            // can we use ringbuffer here?
            memcpy(socket->rcv_buffer_, socket->rcv_buffer_ + i, socket->next_rcv_valid_index_ - i);
            socket->next_rcv_valid_index_ -= i;
        }
    }

    auto OrderServer::recvFinishedCallback() noexcept -> void{
        //fifo_sequencer.sequenceAndPublish();
    }

    OrderServer::OrderServer(ClientRequestLFQueue* client_requests, ClientResponseLFQueue* client_responses, const std::string &iface, int port)
        : iface_(iface), port_(port), client_responses_(client_responses), logger(order_server_log_file), tcp_server_(logger) 
    { // Add init for fifo seq
        clientId_outgoing_seqNum_.fill(1);
        clientId_incoming_seqNum_.fill(1);
        clientId_socket_.fill(nullptr);

        // Lambda
        tcp_server_.recv_callback_ = [this](auto socket, auto rx_time){
            //recvCallback(socket, rx_time);
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