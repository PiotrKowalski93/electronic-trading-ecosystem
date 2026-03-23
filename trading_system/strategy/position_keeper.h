#pragma once

#include "common_types.h"
#include "logging.h"
#include "position_info.h"

using namespace Common;

namespace TradingSystem{

    // PositionKeeper class manages the position and PnL across all trading instruments in the trading engine
    class PositionKeeper{
        public:
            PositionKeeper(Common::Logger* logger): logger_(logger){}

            // Deleted default, copy & move constructors and assignment-operators.
            PositionKeeper() = delete;
            PositionKeeper(const PositionKeeper &) = delete;
            PositionKeeper(const PositionKeeper &&) = delete;
            PositionKeeper &operator=(const PositionKeeper &) = delete;
            PositionKeeper &operator=(const PositionKeeper &&) = delete;

        private:
            std::string time_str_;
            Common::Logger* logger_;

            std::array<PositionInfo, ME_MAX_TICKERS> ticker_position_;

        public: 
            auto addFill(const Exchange::MEClientResponse *client_response) noexcept {
                ticker_position_.at(client_response->tickerId_).addFill(client_response, logger_);
            }

            auto updateBBO(TickerId ticker_id, const BestBidOffer *bbo) noexcept {
                ticker_position_.at(ticker_id).updateBBO(bbo, logger_);
            }

            auto getPositionInfo(TickerId ticker_Id) const noexcept{
                return &ticker_position_.at(ticker_Id);
            }

            auto toString() const {
                double total_pnl = 0;
                Qty total_vol = 0;

                std::stringstream ss;
                for(TickerId i = 0; i < ticker_position_.size(); ++i) {
                    ss << "TickerId:" << tickerIdToString(i) << " " << ticker_position_.at(i).toString() << "\n";

                    total_pnl += ticker_position_.at(i).total_PnL_;
                    total_vol += ticker_position_.at(i).volume_;
                }
                ss << "Total PnL:" << total_pnl << " Vol:" << total_vol << "\n";

                return ss.str();
            }
    };
}