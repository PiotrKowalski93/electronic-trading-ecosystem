#pragma once

#include "macros.h"
#include "logging.h"
#include "common_types.h"
#include "market_order_book.h"

using namespace Common;

namespace TradingSystem{
    constexpr auto Feature_INVALID = std::numeric_limits<double>::quiet_NaN();

    class FeatureEngine {
        public:
            FeatureEngine(Logger* logger);
            ~FeatureEngine();

            auto getMarketPrice() const noexcept;
            auto getAggregatedTradeQtyRatio() const noexcept;
            auto onOrderBookUpdate(TickerId tickerId, Price price, Side side, MarketOrderBook* orderBook) noexcept -> void;

        private:
            std::string time_str_;
            Common::Logger* logger_;

            double market_price_ = Feature_INVALID;
            double aggregated_trade_qty_ratio_ = Feature_INVALID;
    };
}