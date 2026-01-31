#pragma once

#include <array>
#include <sstream>

#include "common/common_types.h"
#include "me_order.h"

using namespace Common;

namespace Exchange {
    auto MEOrder::toString() const -> std::string{
        std::stringstream ss;
        ss << "MEOrder ["
            << "tickerId:" << tickerIdToString(tickerId_)
            << "clientId:" << clientIdToString(clientId_)
            << "client oId:" << orderIdToString(client_orderId_)
            << "market oId:" << orderIdToString(market_orderId_)
            << "side:" << sideToString(side_)
            << "price:" << priceToString(price_)
            << "qty:" << qtyToString(qty_)
            << "priority:" << priorityToString(priority_)
            << "prev:" << orderIdToString(prev_order ? prev_order->market_orderId_ : OrderId_INVALID)
            << "next:" << orderIdToString(next_order ? next_order->market_orderId_ : OrderId_INVALID)
            << " ]";

        return ss.str();
    }

    auto MEOrderAtPriceLevel::toString() const -> std::string{
        std::stringstream ss;
        ss << "MEOrder ["
            << "side:" << sideToString(side_)
            << "price:" << priceToString(price_)
            << "first_order:" << (first_Order_ ? first_Order_->toString() : "null")
            << "prev lvl:" << priceToString(prev_price_level_ ? prev_price_level_->price_ : Price_INVALID)
            << "next lvl" << priceToString (next_price_level_ ? prev_price_level_->price_ : Price_INVALID)
            << " ]";

        return ss.str();
    }
};
