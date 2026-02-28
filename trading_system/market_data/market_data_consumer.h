#pragma once

#include <map>
#include "market_update.h"
#include "multicast_socket.h"

namespace TradingSystem {
    class MarketDataConsumer {
    public:
        Logger logger_;
        std::string time_str_;

        std::atomic<bool> is_running_{false};

        Exchange::MarketDataLFQueue marked_updates_;
        size_t next_update_seq_num_ = 0;
        Common::MulticastSocket incremental_mcast_socket_;
        Common::MulticastSocket snapshot_mcast_socket_;

        QueuedMarketUpdates incremental_queued_market_updates_;

        // Snapshot
        bool in_recovery_mode_ = false;

        size_t iface_;
        std::string snapshot_mcast_ip_;
        int snapshot_mcast_port_;
        QueuedMarketUpdates snapshot_queued_market_updates_;

        MarketDataConsumer() = default;
        ~MarketDataConsumer() = default;
    };

    // map is not optimized for lookups, but we need to store updates 
    // that arrive out of order during recovery, also during recovery trading is halted
    typedef std::map<size_t, Exchange::MEMarketUpdate> QueuedMarketUpdates;
}