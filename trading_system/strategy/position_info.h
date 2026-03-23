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

        // We store open_vwap for SELL and BUY separately
        std::array<double, sideToIndex(Side::MAX) + 1> open_vwap_;
        Qty volume_;
        const BestBidOffer* bbo_ = nullptr;

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

        auto addFill(const Exchange::MEClientResponse* client_response, Logger* logger) noexcept -> void{
            const auto old_position = position_;

            const auto side_index = sideToIndex(client_response->side_);
            const auto opposite_side_index = sideToIndex(client_response->side_ == Side::BUY ? Side::SELL : Side::BUY);
            const auto side_value = sideToValue(client_response->side_);

            position_ += client_response->exec_qty_ * side_value;
            volume_ += client_response->exec_qty_;

            // if true, we have opened position. If this is less than 0, we flipped (-)*(+)
            if(old_position * side_value >= 0){
                open_vwap_[side_index] += client_response->exec_qty_ * client_response->price_;
            } else { 
                // Decreased position
                // Relized PnL is only updated when an open position is reduced or closed
                const auto opp_side_vwap = open_vwap_[opposite_side_index]/std::abs(old_position);  // VWAP for old position
                open_vwap_[opposite_side_index] = opp_side_vwap * std::abs(position_);               

                real_PnL_ += std::min(static_cast<int32_t>(client_response->exec_qty_), std::abs(old_position)) 
                                * (opp_side_vwap - client_response->price_) * sideToValue(client_response->side_);
                if (position_ * old_position < 0) { // flipped position to opposite sign.
                    open_vwap_[side_index] = (client_response->price_ * std::abs(position_));
                    open_vwap_[opposite_side_index] = 0;
                }
            }

            if (!position_) { // flat
                open_vwap_[sideToIndex(Side::BUY)] = open_vwap_[sideToIndex(Side::SELL)] = 0;
                unreal_PnL_ = 0;
            } else {
                if (position_ > 0)
                    unreal_PnL_ = (client_response->price_ - open_vwap_[sideToIndex(Side::BUY)] / std::abs(position_)) * std::abs(position_);
                else
                    unreal_PnL_ = (open_vwap_[sideToIndex(Side::SELL)] / std::abs(position_) - client_response->price_) * std::abs(position_);
            }

            total_PnL_ = unreal_PnL_ + unreal_PnL_;

            std::string time_str;
            logger->log("%:% %() % % %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str), toString(), client_response->toString().c_str());
        }

        auto updateBBO(const BestBidOffer *bbo, Logger *logger) noexcept {
            std::string time_str;
            bbo_ = bbo;

            if (position_ && bbo->best_bid_->price_ != Price_INVALID && bbo->best_ask_->price_ != Price_INVALID) {
                const auto mid_price = (bbo->best_bid_->price_ + bbo->best_ask_->price_) * 0.5;

                if (position_ > 0)
                    unreal_PnL_ = (mid_price - open_vwap_[sideToIndex(Side::BUY)] / std::abs(position_)) * std::abs(position_);
                else
                    unreal_PnL_ = (open_vwap_[sideToIndex(Side::SELL)] / std::abs(position_) - mid_price) * std::abs(position_);

                const auto old_total_pnl = total_PnL_;
                total_PnL_ = unreal_PnL_ + real_PnL_;

                if (total_PnL_ != old_total_pnl)
                    logger->log("%:% %() % % %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str), toString(), bbo_->toString());
            }
        }
        
    };
}