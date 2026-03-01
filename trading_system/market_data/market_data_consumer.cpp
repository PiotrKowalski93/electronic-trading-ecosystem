#include "market_data_consumer.h"

namespace TradingSystem {
    
    MarketDataConsumer::MarketDataConsumer(Common::ClientId client_id, Exchange::MarketDataLFQueue* marked_updates, std::string iface, 
        const std::string& snapshot_mcast_ip, int snapshot_mcast_port, const std::string& incremental_mcast_ip, int incremental_mcast_port)
        : incoming_marked_updates_(marked_updates), is_running_(false), logger_("market_data_consumer_" + std::to_string(client_id) + ".log"),
            incremental_mcast_socket_(logger_), snapshot_mcast_socket_(logger_), iface_(iface),  snapshot_mcast_ip_(snapshot_mcast_ip), 
            snapshot_mcast_port_(snapshot_mcast_port)
    {
        auto recv_callback = [this](auto socket){
            //recvCallback(socket);
        };

        incremental_mcast_socket_.recv_callback_ = recv_callback;
        
        // Init mcast socket for incremental Market Updates
        ASSERT(incremental_mcast_socket_.init(incremental_mcast_ip, iface, incremental_mcast_port, true )>= 0, 
            "Unable to init socket for incremental market updates. Error: " + std::string(std::strerror(errno))); 

        // Join mcast group for Market Updates
        ASSERT(incremental_mcast_socket_.join(incremental_mcast_ip) >= 0,
            "Unable to join mcast on: " + std::to_string(incremental_mcast_socket_.socket_fd_) + ". error: " + std::string(std::strerror(errno)));

        snapshot_mcast_socket_.recv_callback_ = recv_callback;
    }

    auto MarketDataConsumer::run() noexcept -> void {
        logger_.log("%:% %() %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));

        while(is_running_){
            incremental_mcast_socket_.sendAndRecv();
            
            //snapshot_mcast_socket_.sendAndRecv();
        }        
    }
}
