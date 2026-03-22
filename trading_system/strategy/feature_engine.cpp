#include "feature_engine.h"

namespace TradingSystem {
    FeatureEngine::FeatureEngine(Logger* logger) : logger_(logger){}

    auto FeatureEngine::getMarketPrice() const noexcept {
        return market_price_;
    }

    auto FeatureEngine::getAggregatedTradeQtyRatio() const noexcept {
        return aggregated_trade_qty_ratio_;
    }

    auto FeatureEngine::onOrderBookUpdate(TickerId tickerId, Price price, Side side, const MarketOrderBook* orderBook) noexcept -> void{
        const auto bbo = orderBook->getBBO();

        if(LIKELY(bbo->best_ask_->price_ != Price_INVALID && bbo->best_bid_->price_ != Price_INVALID)){
            // Calculate market price
            market_price_ = (bbo->best_bid_->price_ * bbo->best_ask_->total_qty_ + bbo->best_ask_->price_ * bbo->best_bid_->total_qty_)/
                static_cast<double>(bbo->best_bid_->total_qty_ + bbo->best_ask_->total_qty_);
            
            
        logger_->log("%:% %() % ticker:% price:% side:% mkt-price:% agg-trade-ratio:%\n", __FILE__, __LINE__, __FUNCTION__,
                    Common::getCurrentTimeStr(&time_str_), tickerId, Common::priceToString(price).c_str(),
                    Common::sideToString(side).c_str(), market_price_, aggregated_trade_qty_ratio_);    
        }
    }

    auto FeatureEngine::onTradeUpdate(const Exchange::MEMarketUpdate* market_update, const MarketOrderBook* orderBook) noexcept -> void{
        const auto bbo = orderBook->getBBO();

        if(LIKELY(bbo->best_ask_->price_ != Price_INVALID && bbo->best_bid_->price_ != Price_INVALID)){

            // We use static_cast<double> to ensure precision, result must be not int
            aggregated_trade_qty_ratio_ = static_cast<double>(market_update->qty_) /
                 (market_update->side_ == Side::BUY ? bbo->best_ask_->total_qty_ : bbo->best_bid_->total_qty_);
        }

        logger_->log("%:% %() % % mkt-price:% agg-trade-ratio:%\n", __FILE__, __LINE__, __FUNCTION__,
                   Common::getCurrentTimeStr(&time_str_),
                   market_update->toString().c_str(), market_price_, aggregated_trade_qty_ratio_);
    }
}