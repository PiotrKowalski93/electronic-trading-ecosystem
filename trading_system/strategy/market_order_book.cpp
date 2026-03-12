#include "market_order_book.h"

namespace TradingSystem{
    MarketOrderBook::MarketOrderBook(TickerId tickerId, Logger* logger)
        : tickeId_(tickerId), logger_(logger), price_level_pool_(ME_MAX_PRICE_LEVELS), order_pool_(ME_MAX_ORDER_IDS){}

    MarketOrderBook::~MarketOrderBook(){
        trade_engine_ = nullptr;
        bid_price_levels_ = nullptr;
        ask_price_levels_ = nullptr;
        orderId_to_order_.fill(nullptr);
    }

    auto MarketOrderBook::setTradeEngine(TradeEngine* trade_engine) -> void{
        trade_engine_ = trade_engine;
    }

    auto MarketOrderBook::onMarketUpdate(const Exchange::MEMarketUpdate* market_update) noexcept -> void{

        // Check if we have to update BBO
        const auto best_bid_updated = (bid_price_levels_ && market_update->side_ == Side::BUY && market_update->price_ >= bid_price_levels_->price_);
        const auto best_ask_updated = (ask_price_levels_ && market_update->side_ == Side::SELL && market_update->price_ <= bid_price_levels_->price_);

        // Handle market update message
        switch (market_update->type_)
        {
            case Exchange::MarketUpdateType::ADD:{
                // Allocate new MarketOrder
                auto order = order_pool_.allocate(market_update->orderId_, market_update->side_, 
                    market_update->price_, market_update->qty_, market_update->priority_, nullptr, nullptr);
                // addOrder(order);
            }
                break;
            case Exchange::MarketUpdateType::MODIFY:{
                auto order = orderId_to_order_.at(market_update->orderId_);
                order->qty_ = market_update->qty_;
            }
                break;
            case Exchange::MarketUpdateType::CANCEL:{
                auto order = orderId_to_order_.at(market_update->orderId_);
                //removeOrder(order);
            }
            // TRADE does not change the book
            case Exchange::MarketUpdateType::TRADE:{
                //trade_engine_->onTradeUpdate(market_update, this);
                return;
            }
                break;
            // We need to clear whole book after packages drop
            case Exchange::MarketUpdateType::CLEAR:{
                    for(auto& order: orderId_to_order_){
                        if(order){
                            order_pool_.deallocate(order);
                            order = nullptr;
                        }
                    }

                    if(bid_price_levels_){
                        for(auto bid_level = bid_price_levels_->next_level_; bid_level != bid_price_levels_; bid_level = bid_price_levels_->next_level_){
                            price_level_pool_.deallocate(bid_level);
                        }
                        price_level_pool_.deallocate(bid_price_levels_);
                    }

                    if(ask_price_levels_){
                        for(auto ask_level = ask_price_levels_->next_level_; ask_level != ask_price_levels_; ask_level = ask_price_levels_->next_level_){
                            price_level_pool_.deallocate(ask_level);
                        }
                        price_level_pool_.deallocate(ask_price_levels_);
                    }

                    bid_price_levels_ = nullptr;
                    ask_price_levels_ = nullptr;
                }            
                break;
            case Exchange::MarketUpdateType::INVALID:
            case Exchange::MarketUpdateType::SNAPSHOT_START:
            case Exchange::MarketUpdateType::SNAPSHOT_END:
                break;
        }

        //updateBBO(best_bid_updated, best_ask_updated);
    }
}