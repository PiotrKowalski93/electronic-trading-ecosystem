#pragma once

#include <functional>

#include "market_update.h"
#include "logging.h"
#include "multicast_socket.h"

//#include "snapshot_synthesizer.h"

namespace Exchange{
    class MarketDataPublisher{
    public:
        MarketDataPublisher(MarketDataLFQueue* outgoing_md_updates, const std::string &iface, 
            const std::string &snapshot_ip, const int snapshot_port, 
            const std::string &incremental_ip, const int incremental_port);
        ~MarketDataPublisher();

        MarketDataPublisher() = delete;
        MarketDataPublisher(const MarketDataPublisher &) = delete;
        MarketDataPublisher(const MarketDataPublisher &&) = delete;
        MarketDataPublisher &operator=(const MarketDataPublisher &) = delete;
        MarketDataPublisher &operator=(const MarketDataPublisher &&) = delete;  

        auto start() -> void;
        auto stop() -> void;
        auto run() noexcept -> void;

    private:
        size_t next_inc_seq_num_ = 1;
        MarketDataLFQueue* outgoing_md_updates_ = nullptr;
        MDPMarketDataLFQueue snapshot_updates_;

        std::atomic<bool> is_running_ = false;

        std::string time_str_;
        Logger logger_;

        Common::MulticastSocket incremental_socket_;
        //SnapshotSynthesizer* snapshot_synthesizer_ = nullptr;
    };
}