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

    auto MarketOrderBook::addOrdersAtPriceLevel(MarketOrderPriceLevel* price_level) noexcept -> void{
        orders_at_price_levels_.at(priceToIndex(price_level->price_)) = price_level;

        // Get head of double linked-list asks or bids
        auto side_orders_by_price_head = (price_level->side_ == Side::BUY) ? bid_price_levels_ : ask_price_levels_;

        if(UNLIKELY(!side_orders_by_price_head)){
            // no price levels, need to create
            side_orders_by_price_head = price_level;
            side_orders_by_price_head->prev_level_ = price_level;
            side_orders_by_price_head->next_level_ = price_level;
            return;
        }

        auto current_level = side_orders_by_price_head;

        // I changed code from book. It was too complicated to maintain.
        if(price_level->side_ == Side::BUY){
            while(price_level->price_ > current_level->price_ && current_level->next_level_ != side_orders_by_price_head){
                current_level = current_level->next_level_;
            }

            price_level->next_level_ = current_level;
            price_level->prev_level_ = current_level->prev_level_;
            current_level->prev_level_->next_level_ = price_level;
            current_level->prev_level_ = price_level;

            if(price_level->price_ > side_orders_by_price_head->price_){
                bid_price_levels_ = price_level;
            }

            // Update BBO for BID / BUY
            if(!best_offer_.best_bid_ || price_level->price_ > best_offer_.best_bid_->price_){
                best_offer_.best_bid_ = price_level;
            }

        }else{
            while(price_level->price_ < current_level->price_ && current_level->next_level_ != side_orders_by_price_head){
                current_level = current_level->next_level_;
            }

            price_level->next_level_ = current_level;
            price_level->prev_level_ = current_level->prev_level_;
            current_level->prev_level_->next_level_ = price_level;
            current_level->prev_level_ = price_level;

            if(price_level->price_ < side_orders_by_price_head->price_){
                ask_price_levels_ = price_level;
            }

            // Update BBO for ASK / SELL
            if(!best_offer_.best_ask_ || price_level->price_ < best_offer_.best_ask_->price_){
                best_offer_.best_ask_ = price_level;
            }
        }
    }

    auto MarketOrderBook::addOrder(MarketOrder* order) noexcept -> void{
        auto price_level = getPriceLevel(order->price_);

        // PERF: In real life 90% of operations changes aleady existing Price Level
        // added LIKELY first -> most likely hot path
        if(LIKELY(price_level)){
            // Append to price level
            auto first_order = price_level->first_order_;

            // Place order at the end
            first_order->prev_order_->next_order_ = order;
            order->prev_order_ = first_order->prev_order_;
            order->next_order_ = first_order;
            first_order->prev_order_ = order; 
        }else{
            // Create new Price Level
            order->next_order_ = order->prev_order_ = order;

            price_level = price_level_pool_.allocate(order->side_, order->price_, order, nullptr, nullptr);
            addOrdersAtPriceLevel(price_level);
        }

        // Update Price Level Qty and Count
        price_level->total_qty_ += order->qty_;
        ++price_level->orders_count_;
    }
    
    auto MarketOrderBook::removePriceLevel(MarketOrderPriceLevel* price_level) noexcept -> void{
        auto orders_head = (price_level->side_ == Side::BUY ? bid_price_levels_ : ask_price_levels_);
        
        // Price level map is looped, next == current then it is last prive level
        // but we need to null head
        if(UNLIKELY(price_level->next_level_ == price_level)){
            orders_head = nullptr;
        } else {
            price_level->prev_level_->next_level_ = price_level->next_level_;
            price_level->next_level_->prev_level_ = price_level->prev_level_;
            
            //If we remove at head, change head
            if(orders_head == price_level){
                orders_head = price_level->next_level_;
            }
        }

        orders_at_price_levels_.at(priceToIndex(price_level->price_)) = nullptr;
        price_level_pool_.deallocate(price_level);
    }

    auto MarketOrderBook::removeOrder(MarketOrder* order) noexcept -> void{
        auto price_level = getPriceLevel(order->price_);

        // If it points to itself = only one element, we can remove price level
        if(order->prev_order_ == order){
            removePriceLevel(price_level);
        } else {
            const auto prev_order = order->prev_order_;
            const auto next_order = order->next_order_;
            prev_order->next_order_ = next_order;
            next_order->prev_order_ = prev_order;

            //If order was head, set new head
            if(price_level->first_order_ == order){
                price_level->first_order_ = next_order;
            }
        }
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
                addOrder(order);
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

        // trade_engine_->onOrderBookUpdate
        // LOG
    }

    // const noexcept -> does not change class fields
    // const BestBidOffer* -> returns type that cannot be changed
    auto MarketOrderBook::getBBO() const noexcept -> const BestBidOffer*{
        return &best_offer_;
    }
}