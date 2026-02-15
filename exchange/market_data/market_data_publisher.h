#pragma once

#include <functional>

#include "market_update.h"
#include "logging.h"

//#include "snapshot_synthesizer.h"

namespace Exchange{
    class MarketDataPublisher{
    private:
        size_t next_inc_seq_num_ = 1;
        MarketDataLFQueue* outgoing_md_updates = nullptr;
        MDPMarketDataLFQueue* snapshot_updates_ = nullptr;

        std::atomic<bool> is_running_ = false;

        std::string time_str_;
        Logger logger_;

        //Common::MCastSocket incremental_socket_;
        //SnapshotSynthesizer* snapshot_synthesizer_ = nullptr;
    };
}