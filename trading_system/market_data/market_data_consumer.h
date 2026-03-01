#pragma once

#include <map>
#include <cstring>

#include "market_update.h"
#include "multicast_socket.h"

namespace TradingSystem {
    class MarketDataConsumer {
    public:
        Logger logger_;
        std::string time_str_;

        std::atomic<bool> is_running_{false};

        Exchange::MarketDataLFQueue* incoming_marked_updates_ = nullptr;
        size_t next_update_seq_num_ = 0;
        Common::MulticastSocket incremental_mcast_socket_;
        Common::MulticastSocket snapshot_mcast_socket_;

        QueuedMarketUpdates incremental_queued_market_updates_;

        // Snapshot
        bool in_recovery_mode_ = false;

        std::string iface_;
        std::string snapshot_mcast_ip_;
        int snapshot_mcast_port_;
        QueuedMarketUpdates snapshot_queued_market_updates_;

        MarketDataConsumer(Common::ClientId client_id, Exchange::MarketDataLFQueue* marked_updates, std::string iface, 
            const std::string& snapshot_mcast_ip,  int snapshot_mcast_port, const std::string& incremental_mcast_ip,
            int incremental_mcast_port);

        ~MarketDataConsumer(){
            stop();

            using namespace std::literals::chrono_literals;
            std::this_thread::sleep_for(3s);
        }

        auto start() -> void {
            is_running_ = true;

            ASSERT(Common::createAndStartThread(-1, "MarketDataConsumer",
               [this](){ run(); }) >= 0, "Failed to start thread for MarketDataConsumer");
        };

        auto run() noexcept -> void;
        
        auto stop() -> void{
            is_running_ = false;
        }
    };

    // map is not optimized for lookups, but we need to store updates 
    // that arrive out of order during recovery, also during recovery trading is halted
    typedef std::map<size_t, Exchange::MEMarketUpdate> QueuedMarketUpdates;
}