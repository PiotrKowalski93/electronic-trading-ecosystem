#include "feature_engine.h"

namespace TradingSystem {
    FeatureEngine::FeatureEngine(Logger* logger) : logger_(logger){}

    auto FeatureEngine::getMarketPrice() const noexcept {
        return market_price_;
    }

    auto FeatureEngine::getAggregatedTradeQtyRatio() const noexcept {
        return aggregated_trade_qty_ratio_;
    }

    auto FeatureEngine::onOrderBookUpdate(TickerId tickerId, Price price, Side side, MarketOrderBook* orderBook) noexcept -> void{
        const auto bbo = orderBook->getBBO();

        //TODO: Implement
    }
}