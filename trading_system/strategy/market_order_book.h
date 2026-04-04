#pragma once

#include "logging.h"
#include "mem_pool.h"
#include "market_order.h"
#include "market_update.h"

namespace TradingSystem{
    // TODO: To implement
    class TradeEngine{
        public:
            ClientId clientId_;
    };
    
    class MarketOrderBook final {
        public: 
            MarketOrderBook(TickerId tickerId, Logger* logger);
            ~MarketOrderBook();
        
            auto setTradeEngine(TradeEngine* trade_engine) -> void;
            auto onMarketUpdate(const Exchange::MEMarketUpdate* marketUpdate) noexcept -> void;
        
            auto getBBO() const noexcept -> const BestBidOffer*;

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

            // Get
            auto getPriceLevel(Price price) const noexcept -> MarketOrderPriceLevel*{
                return orders_at_price_levels_.at(priceToIndex(price));
            }

            auto priceToIndex(Price price) const noexcept -> unsigned long{
                return (price % ME_MAX_PRICE_LEVELS);
            }

            // Add
            auto addOrdersAtPriceLevel(MarketOrderPriceLevel* price_level) noexcept -> void;
            auto addOrder(MarketOrder* order) noexcept -> void;

            // Delete
            auto removePriceLevel(MarketOrderPriceLevel* price_level) noexcept -> void;
            auto removeOrder(MarketOrder* order) noexcept -> void;
    };

    typedef std::array<MarketOrderBook*, ME_MAX_TICKERS> MarketOrderBooksHashMap;
}

