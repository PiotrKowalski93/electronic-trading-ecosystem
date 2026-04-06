#pragma once

#include "market_order_book.h"
#include "order_manager_order.h"
#include "client_response.h"
#include "client_request.h"
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

            auto moveOrders(TickerId tickerid, Price bid_price, Price ask_price, Qty clip) noexcept -> void{
                auto bid_order = &(ticker_orders_.at(tickerid).at(sideToIndex(Side::BUY)));
                moveOrder(bid_order, tickerid, bid_price, Side::BUY, clip);

                auto ask_order = &(ticker_orders_.at(tickerid).at(sideToIndex(Side::SELL)));
                moveOrder(ask_order, tickerid, ask_price, Side::SELL, clip);
            }

            auto onOrderUpdate(Exchange::MEClientResponse* response) noexcept -> void{
                logger_->log("%:% %() % %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), response->toString().c_str());

                auto order = &(ticker_orders_.at(response->tickerId_).at(sideToIndex(response->side_)));

                logger_->log("%:% %() % %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), order->toString().c_str());

                switch (response->type_) {
                    case Exchange::ClientResponseType::ACCEPTED: {
                        order->order_state_ = OMOrderState::LIVE;
                    }
                    break;
                    case Exchange::ClientResponseType::CANCELED: {
                        order->order_state_ = OMOrderState::DEAD;
                    }
                    break;
                    case Exchange::ClientResponseType::FILLED: {
                        order->qty_ = response->left_qty_;
                        if(!order->qty_) order->order_state_ = OMOrderState::DEAD;
                    }
                    break;
                    case Exchange::ClientResponseType::CANCEL_REJECTED:
                    case Exchange::ClientResponseType::INVALID: {
                    }
                    break;
                }
            }

             // Deleted default, copy & move constructors and assignment-operators.
            OrderManager() = delete;
            OrderManager(const OrderManager &) = delete;
            OrderManager(const OrderManager &&) = delete;
            OrderManager &operator=(const OrderManager &) = delete;
            OrderManager &operator=(const OrderManager &&) = delete;

        private:
            TradeEngine* trade_engine_ = nullptr;
            // const RiskManager& risk_manager_;

            std::string time_str_;
            Common::Logger* logger_ = nullptr;

            OMOrderTickerSideHashMap ticker_orders_;
            OrderId next_orderId_ = 1;

            auto newOrder(OMOrder* order, TickerId tickerId, Price price, Side side, Qty qty) noexcept -> void;
            auto cancelOrder(OMOrder* order) noexcept -> void;
            auto moveOrder(OMOrder* order, TickerId tickerId, Price price, Side side, Qty qty) noexcept -> void;
    };
}