#include "order_manager.h"
#include "client_request.h"

namespace TradingSystem {

    auto OrderManager::newOrder(OMOrder* order, TickerId tickerId, Price price, Side side, Qty qty) noexcept -> void{
        const Exchange::MEClientRequest new_request{
            Exchange::ClientRequestType::NEW,
            trade_engine_->clientId_,
            tickerId,
            next_orderId_,
            price,
            qty,
            side
        };

        // trade_engine_->sendClientRequest(&new_request);

        *order = {tickerId, next_orderId_, side, price, qty, OMOrderState::PENDING_NEW};
        ++next_orderId_;

        logger_->log("%:% %() % Sent new order % for %\n", __FILE__, __LINE__, __FUNCTION__,
                 Common::getCurrentTimeStr(&time_str_),
                 new_request.toString().c_str(), order->toString().c_str());
    }

    auto OrderManager::cancelOrder(OMOrder* order) noexcept -> void{
        const Exchange::MEClientRequest cancel_request{
            Exchange::ClientRequestType::CANCEL,
            trade_engine_->clientId_,
            order->ticker_id_,
            order->order_id_,
            order->price_,
            order->qty_,
            order->side_
        };
        // trade_engine_->sendClientRequest(&cancel_request);

        order->order_state_ = OMOrderState::PENDING_CANCEL;

        logger_->log("%:% %() % Sent cancel % for %\n", __FILE__, __LINE__, __FUNCTION__,
                 Common::getCurrentTimeStr(&time_str_),
                 cancel_request.toString().c_str(), order->toString().c_str());
    }

    auto OrderManager::moveOrder(OMOrder* order, TickerId tickerId, Price price, Side side, Qty qty) noexcept{
        switch (order->order_state_)
        {
            case OMOrderState::LIVE: {
                if(order->price_ != price || order->qty_ != qty){
                    cancelOrder(order);
                }
            }
            break;
            case OMOrderState::INVALID:
            case OMOrderState::DEAD:{
                // const auto risk_result = risk_manager_.checkPreTradeRisk(tickerId, side, qty);
                if(LIKELY(true)){ // use risk_result
                    newOrder(order, tickerId, price, side, qty);
                } else {
                    // logger_->log("%:% %() % Ticker:% Side:% Qty:% RiskCheckResult:%\n", __FILE__, __LINE__, __FUNCTION__,
                    //        Common::getCurrentTimeStr(&time_str_),
                    //        tickerIdToString(ticker_id), sideToString(side), qtyToString(qty),
                    //        riskCheckResultToString(risk_result));
                }
            }
            break;
            case OMOrderState::PENDING_NEW:
            case OMOrderState::PENDING_CANCEL:
            break;
        }
    }

}