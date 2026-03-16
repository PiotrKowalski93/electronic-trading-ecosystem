#include "order_gateway.h"

namespace TradingSystem
{   
    OrderGateway::OrderGateway(ClientId client_id, Exchange::ClientRequestLFQueue* outgoing_requests, Exchange::ClientResponseLFQueue* incoming_responses,
        std::string ip, const std::string iface, const int port): client_id_(client_id), outgoing_requests_(outgoing_requests), incoming_responses_(incoming_responses),
            ip_(ip), iface_(iface), port_(port_), logger_("trading_order_gateway" + std::to_string(client_id) + ".log"), tcp_client_(logger_)
    {
        tcp_client_.recv_callback_ = [this](auto socket, auto rx_time){
            //recvCallback(socket, rx_time);
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
                
                // Send seq Num
                // Send req

                // seq_num++
            }
        }
    }

}