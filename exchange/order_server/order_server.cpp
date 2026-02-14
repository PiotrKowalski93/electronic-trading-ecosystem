#include "order_server.h"

namespace Exchange {
    std::string order_server_log_file = "exchange_order_server.log";

    auto OrderServer::recvCallback(TCPSocket* socket, Nanos rx_time) noexcept -> void{
        logger_.log("%:% %() % Received socket:% len:% rx:%\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_),
                  socket->fd_, socket->next_rcv_valid_index_, rx_time);

        if(socket->next_rcv_valid_index_ >= sizeof(OMClientRequest)){
            size_t i = 0;
            for(; i + sizeof(OMClientRequest) <= socket->next_rcv_valid_index_; i += sizeof(OMClientRequest)) {
                auto request = reinterpret_cast<OMClientRequest*>(socket->rcv_buffer_ + i);
                logger_.log("%:% %() % Received %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), request->toString());

                // Add new socket connection for CliendId
                if(UNLIKELY(clientId_socket_[request->me_client_request.clientId_] == nullptr)){
                    clientId_socket_[request->me_client_request.clientId_] = socket;
                }
                
                //For security reasons we skip message if it comes from different socket for the client we have registered
                //we do not throw on a hot-path
                if(clientId_socket_[request->me_client_request.clientId_] != socket){
                    logger_.log("%:% %() % Received ClientRequest from ClientId:% on different socket:% expected:%\n", __FILE__, __LINE__, __FUNCTION__,
                        Common::getCurrentTimeStr(&time_str_), request->me_client_request.clientId_, socket->fd_,
                        clientId_socket_[request->me_client_request.clientId_]->fd_);
                    //TODO: Send a reject back to the client
                    continue;
                }

                // Check seq number for the request
                auto& next_seq_num = clientId_outgoing_seqNum_[request->me_client_request.clientId_];
                if(request->seq_num != next_seq_num){
                    logger_.log("%:% %() % Incorrect sequence number. ClientId:% SeqNum expected:% received:%\n", __FILE__, __LINE__, __FUNCTION__,
                        Common::getCurrentTimeStr(&time_str_), request->me_client_request.clientId_, next_seq_num, request->seq_num);

                    //TODO: Send a reject back to the client
                    continue;
                }

                // increment seq num for this client
                ++next_seq_num;

                fifo_sequencer.addClientRequest(rx_time, request->me_client_request);
            }

            // Remove readed from buffer, use memmove()?
            // can we use ringbuffer here?
            memcpy(socket->rcv_buffer_, socket->rcv_buffer_ + i, socket->next_rcv_valid_index_ - i);
            socket->next_rcv_valid_index_ -= i;
        }
    }

    auto OrderServer::recvFinishedCallback() noexcept -> void{
        fifo_sequencer.sequenceAndPublish();
    }

    OrderServer::OrderServer(ClientRequestLFQueue* client_requests, ClientResponseLFQueue* client_responses, const std::string &iface, int port)
        : iface_(iface), port_(port), client_responses_(client_responses), 
        logger_(order_server_log_file), tcp_server_(logger_), fifo_sequencer(client_requests, &logger_)
    { 
        clientId_outgoing_seqNum_.fill(1);
        clientId_incoming_seqNum_.fill(1);
        clientId_socket_.fill(nullptr);

        // Lambda
        tcp_server_.recv_callback_ = [this](auto socket, auto rx_time){
            recvCallback(socket, rx_time);
        };

        tcp_server_.recv_finished_callback_ = [this](){
            recvFinishedCallback();
        };
    }

    auto OrderServer::start() -> void{
        is_running_ = true;
        tcp_server_.listen(iface_, port_);

        ASSERT(Common::createAndStartThread(-1, "Exchange/OrderServer", [this]() { run(); }) != nullptr, "Failed to start OrderServer thread.");
    }

    // Main run loop for this thread - accepts new client connections, 
    // receives client requests from them and sends client responses to them.
    auto OrderServer::run() noexcept -> void{
        logger_.log("%:% %() %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));

        while(is_running_){
            tcp_server_.poll();
            tcp_server_.sendAndRecv();

            for(auto client_response = client_responses_->getNextToRead(); client_responses_->size() && client_response; client_response = client_responses_->getNextToRead()){
                // Need alias because we will increment it after sending msg
                auto &next_out_seq_num = clientId_outgoing_seqNum_[client_response->clientId_];
                
                ASSERT(clientId_socket_[client_response->clientId_] != nullptr, "There is no TCP Socket for ClientId: " + std::to_string(client_response->clientId_));
                
                // Send seq num
                clientId_socket_[client_response->clientId_]->send(&next_out_seq_num, sizeof(next_out_seq_num));

                // Send response
                clientId_socket_[client_response->clientId_]->send(client_response, sizeof(client_response));

                client_responses_->updateReadIndex();
                ++next_out_seq_num;
            }
        }
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