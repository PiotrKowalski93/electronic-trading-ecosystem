#pragma once

#include <array>
#include <sstream>

#include "common_types.h"

using namespace Common;

namespace Exchange {

    struct MEOrder {
        TickerId tickerId_ = TickerId_INVALID;
        ClientId clientId_ = ClientId_INVALID;
        OrderId client_orderId_ = OrderId_INVALID;
        OrderId market_orderId_ = OrderId_INVALID;
        Side side_ = Side::INVALID;
        Price price_ = Price_INVALID;
        Qty qty_ = Qty_INVALID;
        Priority priority_ = Priority_INVALID;

        MEOrder *prev_order_ = nullptr;
        MEOrder *next_order_ = nullptr;

        // Needen for use with MemPool
        MEOrder() = default;

        MEOrder(TickerId tickerId_, ClientId clientId_, OrderId client_orderId_, OrderId market_orderId_,
        Side side_, Price price_, Qty qty_, Priority priority_, MEOrder *prev_order, MEOrder *next_order) noexcept 
            : tickerId_(tickerId_), clientId_(clientId_), client_orderId_(client_orderId_), market_orderId_(market_orderId_),
            side_(side_), price_(price_), qty_(qty_), priority_(priority_), prev_order_(prev_order), next_order_(next_order)
        {};

        auto toString() const -> std::string;
    };

        // array not map() -> understand why
        typedef std::array<MEOrder*, ME_MAX_ORDER_IDS> OrderHashMap;
        typedef std::array<OrderHashMap, ME_MAX_NUM_CLIENTS> ClientOrderHashMap;

    struct MEOrderAtPriceLevel {
        Side side_ = Side::INVALID;
        Price price_ = Price_INVALID;

        MEOrder *first_order_ = nullptr;

        MEOrderAtPriceLevel *prev_price_level_ = nullptr;
        MEOrderAtPriceLevel *next_price_level_ = nullptr;

        MEOrderAtPriceLevel() = default;

        MEOrderAtPriceLevel(Side side, Price price, MEOrder *first_order, 
                MEOrderAtPriceLevel *prev_price_level, MEOrderAtPriceLevel *next_price_level) noexcept 
            : side_(side), price_(price), first_order_(first_order), prev_price_level_(prev_price_level), next_price_level_(next_price_level)
            {};

        auto toString() const -> std::string;
    };

    typedef std::array<MEOrderAtPriceLevel*, ME_MAX_PRICE_LEVELS> OrdersAtPriceLevelHashMap;
}