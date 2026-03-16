#include "order_gateway.h"

namespace TradingSystem
{   
    auto OrderGateway::recvCallback(TCPSocket* socket, Nanos rx_time) noexcept -> void{
        logger_.log("%:% %() % Received on socket: % len: % %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), 
                    socket->fd_, socket->next_rcv_valid_index_, rx_time);

        // Check size, then we know that we have full msg
        if(socket->next_rcv_valid_index_ >= sizeof(Exchange::OMClientResponse)){
            // i is used to cound read bytes, then we will move them on tcp buffer
            size_t i = 0;

            // We can read more than one response
            for(; i + sizeof(Exchange::OMClientResponse) <= socket->next_rcv_valid_index_;i + sizeof(Exchange::OMClientResponse)){
                auto response = reinterpret_cast<Exchange::OMClientResponse*>(socket->rcv_buffer_ + i);
                logger_.log("%:% %() % Received response: %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), response->toString());

                // Book says that we need to check clientId on response. Wierd that exchange might send other's client response
                if(response->me_client_response.clientId_ != client_id_){
                    logger_.log("%:% %() % ERROR Incorrect ClientId. Expected: % Reveived: %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_),
                        client_id_, response->me_client_response.clientId_);
                    continue;
                }

                // We need to handle wrong sequential number, for now - log error
                // TODO: How correctly handle such case?
                if(response->seq_num != next_exp_response_seq_num_){
                    logger_.log("%:% %() % ERROR Incorrect SeqNum. Expected: % Reveived: %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_),
                        next_exp_response_seq_num_, response->seq_num);
                    continue;
                }

                // Increment seq num
                ++next_exp_response_seq_num_;

                auto next_write = incoming_responses_->getNextToWriteTo();
                // Move allows us to use move ctor, skip copy
                *next_write = std::move(response->me_client_response);
                incoming_responses_->updateNextToWriteTo();
            }
            // Clear tcp buffor of readed msg's
            memcpy(socket->rcv_buffer_, socket->rcv_buffer_ + i, socket->next_rcv_valid_index_ - i);
            socket->next_rcv_valid_index_ -= i;
        }
    }

    OrderGateway::OrderGateway(ClientId client_id, Exchange::ClientRequestLFQueue* outgoing_requests, Exchange::ClientResponseLFQueue* incoming_responses,
        std::string ip, const std::string iface, const int port): client_id_(client_id), outgoing_requests_(outgoing_requests), incoming_responses_(incoming_responses),
        ip_(ip), iface_(iface), port_(port_), logger_("trading_order_gateway" + std::to_string(client_id) + ".log"), tcp_client_(logger_)
    {
        tcp_client_.recv_callback_ = [this](auto socket, auto rx_time){
            recvCallback(socket, rx_time);
        };
    }

    OrderGateway::~OrderGateway(){
        stop();

        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(5s);
    }

    auto OrderGateway::run() noexcept -> void{
        logger_.log("%:% %() % OrderGateway::run() \n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));
        
        while(is_running_){
            tcp_client_.sendAndRecv();

            for(auto request = outgoing_requests_->getNextToRead(); request; request = outgoing_requests_->getNextToRead()){
                logger_.log("%:% %() % Sending order. ClientId: % SeqNum: % %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), 
                    client_id_, next_outgoing_seq_num_, request->toString());
                
                // Sending seq first allows to validate before parsing (might be costly)
                // should be wrap data in struct { seq; msg; } and send in one send()?
                tcp_client_.send(&next_outgoing_seq_num_, sizeof(next_outgoing_seq_num_));
                tcp_client_.send(request, sizeof(Exchange::MEClientRequest));

                outgoing_requests_->updateReadIndex();
                next_outgoing_seq_num_++;
            }
        }
    }

}