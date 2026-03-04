#include "market_data_consumer.h"

namespace TradingSystem {
    
    auto MarketDataConsumer::queueMessage(bool is_snapshot, const Exchange::MDPMarketUpdate* request) -> void{
        if(is_snapshot){
            if(snapshot_queued_market_updates_.find(request->seq_num) != snapshot_queued_market_updates_.end()){
                logger_.log("%:% %() % Packages drops on snapshot socket.\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));
                snapshot_queued_market_updates_.clear();
            }

            snapshot_queued_market_updates_[request->seq_num] = request->me_market_update;
        } else{
            incremental_queued_market_updates_[request->seq_num] = request->me_market_update;
        }

        // checkSnapshotSync();
    }

    auto MarketDataConsumer::startSnapshotSync() -> void{
        incremental_queued_market_updates_.clear();
        snapshot_queued_market_updates_.clear();

        ASSERT(snapshot_mcast_socket_.init(snapshot_mcast_ip_, iface_, snapshot_mcast_port_, true) >= 0, "Unable to init socket for snapshot mcast.");
        ASSERT(snapshot_mcast_socket_.join(snapshot_mcast_ip_) >= 0, "Unable to join mcast group: " + snapshot_mcast_ip_);
    }

    auto MarketDataConsumer::recvCallback(MulticastSocket* socket) noexcept -> void{
        const auto is_snapshot = (socket->socket_fd_ == snapshot_mcast_socket_.socket_fd_);

        // If we got snapshot message not in recovery mode
        if(UNLIKELY(is_snapshot && !in_recovery_mode_)){
           logger_.log("%:% %() % WARN Not expected snapshot msg.\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));
           return;
        }

        // Procesing incremental marked updates
        if(socket->next_rcv_valid_index_ >= sizeof(Exchange::MDPMarketUpdate)){
            size_t i = 0;

            for(; i + sizeof(Exchange::MDPMarketUpdate) <= socket->next_rcv_valid_index_; i += sizeof(Exchange::MDPMarketUpdate)){
                auto request = reinterpret_cast<const Exchange::MDPMarketUpdate*>(socket->next_rcv_valid_index_ + i);
                logger_.log("%:% %() % Received [%]: %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_),
                    (is_snapshot ? "snapshot" : "incremental"), request->toString());

                const bool already_in_recovery_mode = in_recovery_mode_;
                in_recovery_mode_ = (in_recovery_mode_ || request->seq_num != next_update_seq_num_);

                if(UNLIKELY(in_recovery_mode_)){
                    if(UNLIKELY(already_in_recovery_mode)){
                        logger_.log("%:% %() % Packed drops on socket %. SeqNum expected: % recieved: %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), 
                        socket->socket_fd_, next_update_seq_num_, request->seq_num);

                        startSnapshotSync();
                    }
                    queueMessage(is_snapshot, request);

                }else if(!is_snapshot){
                    logger_.log("%:% %() % %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), request->toString());

                    ++next_update_seq_num_;

                    // Save to incoming MD updates LFQueue (for trading engine)
                    auto next_update_write_index = incoming_marked_updates_->getNextToWriteTo();
                    *next_update_write_index = std::move(request->me_market_update);
                    incoming_marked_updates_->updateNextToWriteTo();
                }
            } 
            
            // consume from front of buffer and compact remaining bytes
            memcpy(socket->inbound_data_.data(), socket->inbound_data_.data() + i, socket->next_rcv_valid_index_ - i);
            socket->next_rcv_valid_index_ -= i;
        }
    }

    MarketDataConsumer::MarketDataConsumer(Common::ClientId client_id, Exchange::MarketDataLFQueue* marked_updates, std::string iface, 
        const std::string& snapshot_mcast_ip, int snapshot_mcast_port, const std::string& incremental_mcast_ip, int incremental_mcast_port)
        : incoming_marked_updates_(marked_updates), is_running_(false), logger_("market_data_consumer_" + std::to_string(client_id) + ".log"),
            incremental_mcast_socket_(logger_), snapshot_mcast_socket_(logger_), iface_(iface),  snapshot_mcast_ip_(snapshot_mcast_ip), 
            snapshot_mcast_port_(snapshot_mcast_port)
    {
        auto recv_callback = [this](auto socket){
            recvCallback(socket);
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
