#pragma once

#include "market_order_book.h"
#include "order_manager_order.h"
#include "macros.h"

namespace TradingSystem{
    class OrderManager{
        public:
            OrderManager(Common::Logger *logger, TradeEngine *trade_engine)//, RiskManager& risk_manager)
                : trade_engine_(trade_engine), /*risk_manager_(risk_manager),*/ logger_(logger) {
            }

            // Returns BUY/SELL orders for given ticker
            auto getOMOrderSideHashMap(TickerId ticker_id) const {
                return &(ticker_orders_.at(ticker_id));
            };

        private:
            TradeEngine* trade_engine_ = nullptr;
            // const RiskManager& risk_manager_;

            std::string time_str_;
            Common::Logger* logger_ = nullptr;

            OMOrderTickerSideHashMap ticker_orders_;
            OrderId next_orderId_ = 1;

            auto newOrder(OMOrder* order, TickerId tickerId, Price price, Side side, Qty qty) noexcept -> void;
            auto cancelOrder(OMOrder* order) noexcept -> void;
            auto moveOrder(OMOrder* order, TickerId tickerId, Price price, Side side, Qty qty) noexcept;
    };
}