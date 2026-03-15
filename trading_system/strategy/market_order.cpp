#include "market_order.h"

namespace TradingSystem{
    auto MarketOrder::toString() const -> std::string {
        std::stringstream ss;
        ss << "MarketOrder" << "["
        << "oid:" << orderIdToString(orderId_) << " "
        << "side:" << sideToString(side_) << " "
        << "price:" << priceToString(price_) << " "
        << "qty:" << qtyToString(qty_) << " "
        << "prio:" << priorityToString(priority_) << " "
        << "prev:" << orderIdToString(prev_order_ ? prev_order_->orderId_ : OrderId_INVALID) << " "
        << "next:" << orderIdToString(next_order_ ? next_order_->orderId_ : OrderId_INVALID) << "]";

        return ss.str();
    }

    auto MarketOrderPriceLevel::toString() const -> std::string {
      std::stringstream ss;
      ss << "MarketOrdersAtPrice["
         << "side:" << sideToString(side_) << " "
         << "price:" << priceToString(price_) << " "
         << "first_mkt_order:" << (first_order_ ? first_order_->toString() : "null") << " "
         << "prev:" << priceToString(prev_level_ ? prev_level_->price_ : Price_INVALID) << " "
         << "next:" << priceToString(next_level_ ? next_level_->price_ : Price_INVALID) << "]";

      return ss.str();
    }

    auto BestBidOffer::toString() const -> std::string {
      std::stringstream ss;
      ss << "BBO{"
         << qtyToString(best_bid_->total_qty_) << "@" << priceToString(best_bid_->price_)
         << "X"
         << priceToString(best_ask_->total_qty_) << "@" << qtyToString(best_ask_->price_)
         << "}";

      return ss.str();
    }; 
}