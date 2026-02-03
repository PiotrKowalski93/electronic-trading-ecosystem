#pragma once

#include "common_types.h"
#include "mem_pool.h"
#include "logging.h"
#include "client_response.h"
#include "market_update.h"
#include "matching_engine.h"

#include "me_order.h"

using namespace Common;

namespace Exchange{

    class MEOrderBook final {
    public:
        MEOrderBook(TickerId tickerId, MatchingEngine* matching_engine, Logger* logger);
        ~MEOrderBook();

        MEOrderBook() = delete;
        MEOrderBook(const MEOrderBook&) = delete;
        MEOrderBook(const MEOrderBook&&) = delete;
        MEOrderBook& operator=(const MEOrderBook&) = delete;
        MEOrderBook& operator=(const MEOrderBook&&) = delete;

        auto add(ClientId clientId, OrderId client_orderId, TickerId tickerId, Side side, Price price, Qty qty) noexcept -> void;
        auto getNextPriority(TickerId tickerId, Price price) noexcept -> Priority;
        auto addOrder(MEOrder* order) noexcept -> void;
        auto addOrderAtPrice(MEOrderAtPriceLevel* new_price_level) noexcept -> void;

        // Returns qty that left, if 0, did not matched, if left qty == qty, fully executed
        auto checkForMatch(ClientId clientId, OrderId client_orderId, TickerId tickerId, Side side, Price price, Qty qty, OrderId market_order_Id) noexcept -> Qty;

    private:
        TickerId tickerId_ = TickerId_INVALID;

        MatchingEngine* matching_engine_ = nullptr;

        // typedef std::array<MEOrder*, ME_MAX_ORDER_IDS> OrderHashMap;
        // typedef std::array<OrderHashMap, ME_MAX_NUM_CLIENTS> ClientOrderHashMap;
        ClientOrderHashMap client_orders_;

        MemPool<MEOrderAtPriceLevel> price_levels_pool_;
        MEOrderAtPriceLevel* bids_levels_ = nullptr;
        MEOrderAtPriceLevel* asks_levels_ = nullptr;

        OrdersAtPriceLevelHashMap price_levels_;

        MemPool<MEOrder> order_pool_;

        MEClientResponse client_response_;
        MEMarketUpdate market_update_;

        OrderId next_market_order_id_ = 1;

        std::string time_str_;
        Logger* logger_ = nullptr;

        auto generateNewMarketOrderId() noexcept -> OrderId {
            return next_market_order_id_++;
        }

        auto priceToIndex(Price price) const noexcept -> unsigned long{
            return (price % ME_MAX_PRICE_LEVELS);
        }

        auto getOrdersAtPrcie(Price price) const noexcept -> MEOrderAtPriceLevel*{
            return price_levels_.at(priceToIndex(price));
        }
    };

    typedef std::array<MEOrderBook*, ME_MAX_TICKERS> OrderBookHashMap;
}