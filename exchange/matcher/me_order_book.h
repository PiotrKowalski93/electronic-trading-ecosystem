#pragma once

#include "common_types.h"
#include "mem_pool.h"
#include "logging.h"
#include "client_response.h"
#include "market_update.h"

#include "me_order.h"

using namespace Common;

namespace Exchange{
    // Forward Declaration
    class MatchingEngine;

    // Not final for tests
    class MEOrderBook {
    public:
        MEOrderBook(TickerId tickerId, MatchingEngine* matching_engine, Logger* logger);
        ~MEOrderBook();

        MEOrderBook() = delete;
        MEOrderBook(const MEOrderBook&) = delete;
        MEOrderBook(const MEOrderBook&&) = delete;
        MEOrderBook& operator=(const MEOrderBook&) = delete;
        MEOrderBook& operator=(const MEOrderBook&&) = delete;

        // Every method is public, my junior C++ ass needs to test it
        // public:
        auto add(ClientId clientId, OrderId client_orderId, TickerId tickerId, Side side, Price price, Qty qty) noexcept -> void;
        auto cancel(ClientId clientId, OrderId client_orderId, TickerId tickerId) noexcept -> void;
        auto match(TickerId tickerId, ClientId clientId, Side side, OrderId client_orderId, OrderId market_orderId, MEOrder* itr, Qty* leaves_qty) noexcept -> void;

        // private:
        auto addOrder(MEOrder* order) noexcept -> void;
        auto addOrdersAtPrice(MEOrderAtPriceLevel* new_price_level) noexcept -> void;
        auto getNextPriority(TickerId tickerId, Price price) noexcept -> Priority;
        // Returns qty that left, if 0, did not matched, if left qty == qty, fully executed
        auto checkForMatch(ClientId clientId, OrderId client_orderId, TickerId tickerId, Side side, Price price, Qty qty, OrderId market_order_Id) noexcept -> Qty;
        auto removeOrder(MEOrder* order) noexcept -> void;
        auto removePriceLevel(Side side, Price price) noexcept -> void;

        auto generateNewMarketOrderId() noexcept -> OrderId {
            return next_market_order_id_++;
        }

        auto priceToIndex(Price price) const noexcept -> unsigned long{
            return (price % ME_MAX_PRICE_LEVELS);
        }

        auto getPriceLevel(Price price) const noexcept -> MEOrderAtPriceLevel*{
            return price_levels_.at(priceToIndex(price));
        }

    private:
        TickerId tickerId_ = TickerId_INVALID;

        MatchingEngine* matching_engine_ = nullptr;

        // typedef std::array<MEOrder*, ME_MAX_ORDER_IDS> OrderHashMap;
        // typedef std::array<OrderHashMap, ME_MAX_NUM_CLIENTS> ClientOrderHashMap;
        ClientOrderHashMap client_orders_;

        // Price levels are always kept sorted at insertion time.
        // Matching never reorders price levels.
        // BIDs -> descending
        // ASKs -> ascending
        MEOrderAtPriceLevel* bid_levels_head_ = nullptr;
        MEOrderAtPriceLevel* ask_levels_head_ = nullptr;

        // NOTE:
        // This is a direct-address price table (price ladder), NOT a traditional hashmap.
        // For a given instrument and price, there can be at most ONE active price level.
        //
        // IMPORTANT INVARIANT:
        // BID and ASK price levels never coexist at the same price.
        // If an incoming order reaches an existing opposite-side price level,
        // matching occurs immediately and the price level is consumed/removed.
        // Therefore, a single array indexed by Price is sufficient.
        //
        // The `side_` field indicates whether this price level belongs to BID or ASK,
        // but is NOT meant to allow both sides at the same price simultaneously.
        //
        // This design avoids extra indirection, branching and cache misses,
        // which is critical for low-latency matching engines.
        OrdersAtPriceLevelHashMap price_levels_;
        MemPool<MEOrderAtPriceLevel> price_levels_pool_;

        MemPool<MEOrder> order_pool_;

        MEClientResponse client_response_;
        MEMarketUpdate market_update_;

        OrderId next_market_order_id_ = 1;

        std::string time_str_;
        Logger* logger_ = nullptr;
    };

    typedef std::array<MEOrderBook*, ME_MAX_TICKERS> OrderBookHashMap;
}