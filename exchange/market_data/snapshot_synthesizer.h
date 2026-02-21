#pragma once

#include "common_types.h"
#include "thread_utils.h"
#include "lf_queue.h"
#include "macros.h"
#include "multicast_socket.h"
#include "mem_pool.h"
#include "logging.h"

#include "market_update.h"
#include "me_order.h"

using namespace Common;

namespace Exchange{
    class SnapshotSynthesizer{
    public: 
        SnapshotSynthesizer(MDPMarketDataLFQueue* market_updates, const std::string &iface, const std::string &snapshot_ip, int snapshot_port);
        ~SnapshotSynthesizer();

        //TODO: Why every method is not with noexcept?
        auto start() -> void;
        auto stop() -> void;
        auto run() noexcept -> void;

        auto addToSnapshot(const MDPMarketUpdate* market_update) -> void;

    private:
        MDPMarketDataLFQueue* snapshot_marker_updates_ = nullptr;

        Logger logger_;

        std::atomic<bool> is_running = false;
        std::string time_str_;

        MulticastSocket snapshot_socket_;

        std::array<std::array<MEMarketUpdate*, ME_MAX_ORDER_IDS>, ME_MAX_TICKERS> orders_per_ticker_;
        size_t last_inc_seq_num_ = 0;
        Nanos last_snapshot_time_ = 0;

        MemPool<MEMarketUpdate> order_pool_;
    };
}