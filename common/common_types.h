#pragma once

#include <cstdint>
#include <limits>
#include <iostream>
#include <sstream>

#include "constraints.h"
#include "macros.h"

namespace Common {

    constexpr auto _INVALID = "INVALID";

    typedef uint64_t OrderId;
    constexpr auto OrderId_INVALID = std::numeric_limits<OrderId>::max();

    typedef uint32_t TickerId;
    constexpr auto TickerId_INVALID = std::numeric_limits<TickerId>::max();

    typedef uint32_t ClientId;
    constexpr auto ClientId_INVALID = std::numeric_limits<TickerId>::max();

    typedef uint64_t Price;
    constexpr auto Price_INVALID = std::numeric_limits<Price>::max();

    typedef uint32_t Qty;
    constexpr auto Qty_INVALID = std::numeric_limits<Qty>::max();

    typedef uint64_t Priority;
    constexpr auto Priority_INVALID = std::numeric_limits<Priority>::max();

    enum class Side : int8_t {
        INVALID = 0,
        BUY = 1,
        SELL = -1,
        MAX = 2
    };

    //removed inline, constexpr = inline
    constexpr auto sideToIndex(Side side) noexcept {
        return static_cast<size_t>(side) + 1;
    }

    constexpr auto sideToValue(Side side) noexcept {
        return static_cast<int>(side);
    }

    // -----------

    inline auto orderIdToString(OrderId orderId) -> std::string {
        if(UNLIKELY(orderId == OrderId_INVALID)) return _INVALID; 
        return std::to_string(orderId);
    }

    inline auto tickerIdToString(TickerId tickerId) -> std::string {
        if(UNLIKELY(tickerId == TickerId_INVALID)) return _INVALID;
        return std::to_string(tickerId);
    }

    inline auto clientIdToString(ClientId clientId) -> std::string {
        if(UNLIKELY(clientId == ClientId_INVALID)) return _INVALID;
        return std::to_string(clientId);
    }

    inline auto priceToString(Price price) -> std::string {
        if(UNLIKELY(price == Price_INVALID)) return _INVALID;
        return std::to_string(price);
    }

    inline auto qtyToString(Qty qty) -> std::string {
        if(UNLIKELY(qty == Qty_INVALID)) return _INVALID;
        return std::to_string(qty);
    }

    inline auto priorityToString(Priority priority) -> std::string {
        if(UNLIKELY(priority == Priority_INVALID)) return _INVALID;
        return std::to_string(priority);
    }

    inline auto sideToString(Side side) -> std::string {
        switch (side) {
            case Side::INVALID:
                return _INVALID;
                break;
            case Side::BUY:
                return "BUY";
                break;
            case Side::SELL:
                return "SELL";
                break;
            default:
                break;
        }
        return "UNKNOWN";
    }

    struct RiskCfg{
        Qty max_order_size_ = 0;
        Qty max_position_ = 0;
        double max_loss_ = 0;

        auto toString() const {
            std::stringstream ss;

            ss << "RiskCfg{"
                << "max-order-size:" << qtyToString(max_order_size_) << " "
                << "max-position:" << qtyToString(max_position_) << " "
                << "max-loss:" << max_loss_
                << "}";
            return ss.str();
        }
    };

    struct TradeEngineCfg {
        Qty clip_ = 0;
        double threshold_ = 0;
        RiskCfg risk_cfg_;

        auto toString() const {
            std::stringstream ss;
            ss << "TradeEngineCfg{"
                << "clip:" << qtyToString(clip_) << " "
                << "thresh:" << threshold_ << " "
                << "risk:" << risk_cfg_.toString()
                << "}";

            return ss.str();
        }
    };
    
    typedef std::array<TradeEngineCfg, ME_MAX_TICKERS> TradeEngineCfgHashMap;
}