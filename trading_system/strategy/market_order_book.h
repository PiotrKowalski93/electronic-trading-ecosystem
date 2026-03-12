#pragma once

#include "logging.h"
#include "mem_pool.h"
#include "market_order.h"

namespace TradingSystem{
    class TradeEngine;
    
    class MarketOrderBook final {

        MarketOrderBook();
        ~MarketOrderBook();

        private:
            // Order book is keept ber ticker - ex AAPL
            const TickerId tickeId;

            TradeEngine* trade_engine_ = nullptr;

            // All orders in book by OrderId
            MarketOrderHashMap orderId_to_order_;

            MemPool<MarketOrderPriceLevel> price_level_pool_;
            
            MarketOrderPriceLevel* bid_price_levels_ = nullptr;
            MarketOrderPriceLevel* ask_price_levels_ = nullptr; 
    
            MarketOrderPriceLevelsHashMap orders_at_price_levels_;

            MemPool<MarketOrder> order_pool_;
            BestBidOffer best_offer_;

            std::string time_str_;
            Logger* logger_ = nullptr;
    };

    typedef std::array<MarketOrderBook*, ME_MAX_TICKERS> MarketOrderBooksHashMap;
}

