#pragma once

#include "macros.h"
#include "common_types.h"
#include "logging.h"

#include "client_response.h"
#include "market_order_book.h"

using namespace Common;

namespace TradingSystem{
    struct PositionInfo{
        int32_t position_ = 0;
        double real_PnL_ = 0;
        double unreal_PnL_ = 0;
        double total_PnL_ = 0;

        std::array<double, sideToIndex(Side::MAX) + 1> open_vwap_;
        Qty volume_;
        const BestBidOffer* bbo_ = nullptr;

        auto addFill() -> void{
            //TODO: Implement after you understand VWAP and open_VWAP calculations
        }

        auto toString() const {
            std::stringstream ss;
            ss << "Position{"
                << "pos:" << position_
                << " u-pnl:" << unreal_PnL_
                << " r-pnl:" << real_PnL_
                << " t-pnl:" << total_PnL_
                << " vol:" << qtyToString(volume_)
                << " vwaps:[" << (position_ ? open_vwap_.at(sideToIndex(Side::BUY)) / std::abs(position_) : 0)
                << "X" << (position_ ? open_vwap_.at(sideToIndex(Side::SELL)) / std::abs(position_) : 0)
                << "] "
                << (bbo_ ? bbo_->toString() : "") << "}";

            return ss.str();
        };
    };
}