#pragma once

#include "mem_pool.h"
#include "market_order.h"

namespace TradingSystem{
    class TradeEngine;
    
    class MarketOrderBook final {
        private:
            // Order book is keept ber ticker - ex AAPL
            const TickerId tickeId;

            TradeEngine* trade_engine_ = nullptr;

            // All orders in book by OrderId
            MarketOrderHashMap orderId_to_order_;

            MemPool<MarketOrderPriceLevel> price_level_pool_;
            
    };
}

