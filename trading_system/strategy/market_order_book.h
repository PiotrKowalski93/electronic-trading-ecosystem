#pragma once

#include "logging.h"
#include "mem_pool.h"
#include "market_order.h"
#include "market_update.h"

namespace TradingSystem{
    class TradeEngine;
    
    class MarketOrderBook final {

        MarketOrderBook(TickerId tickerId, Logger* logger);
        ~MarketOrderBook();

        auto setTradeEngine(TradeEngine* trade_engine) -> void;
        auto onMarketUpdate(const Exchange::MEMarketUpdate* marketUpdate) noexcept -> void;

        private:
            // Order book is keept ber ticker - ex AAPL
            const TickerId tickeId_;

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

            auto updateBBO(const bool update_bid, const bool update_ask) -> void;

            // Get
            auto getOrdersAtPriceLevel(Price price) const noexcept -> MarketOrderPriceLevel;
            auto priceToIndex(Price price) const noexcept;

            // Add
            auto addOrder(MarketOrder* order) noexcept -> void;
            auto addOrdersAtPriceLevel(MarketOrderPriceLevel* price_level) noexcept -> void;

            // Delete
            auto removeOrder(MarketOrder* order) noexcept -> void;
            auto removeOrdersAtPriceLevel(Side side, Price price) noexcept -> void;
    };

    typedef std::array<MarketOrderBook*, ME_MAX_TICKERS> MarketOrderBooksHashMap;
}

