#include "me_order_book.h"
#include "matching_engine.h"

namespace Exchange {
    MEOrderBook::MEOrderBook(TickerId tickerId, MatchingEngine* matching_engine, Logger* logger)
        : tickerId_(tickerId), 
            matching_engine_(matching_engine),
            price_levels_pool_(ME_MAX_PRICE_LEVELS),
            order_pool_(ME_MAX_ORDER_IDS),
            logger_(logger){}

    MEOrderBook::~MEOrderBook() {
        logger_->log("%:% %() % ~MEOrderBook\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));

        matching_engine_ = nullptr;
        bid_levels_head = nullptr;
        ask_levels_head = nullptr;

        for (auto &itr: client_orders_) {
            itr.fill(nullptr);
        }
    }

    // TODO: Try to do it brancheless
    auto MEOrderBook::addOrdersAtPrice(MEOrderAtPriceLevel* new_price_level) noexcept -> void{
        price_levels_.at(priceToIndex(new_price_level->price_)) = new_price_level;

        //Get head of double linked-list asks or bids
        auto side_orders_by_price_head = (new_price_level->side_ == Side::BUY) ? bid_levels_head : ask_levels_head;

        if(UNLIKELY(!side_orders_by_price_head)){
            //no price levels, need to create
            side_orders_by_price_head = new_price_level;
            side_orders_by_price_head->prev_price_level_ = new_price_level;
            side_orders_by_price_head->next_price_level_ = new_price_level;
            return;
        }

        auto current_level = side_orders_by_price_head;
        // I changed code from book. It was too complicated to maintain.
        if(new_price_level->side_ == Side::BUY){
            while(new_price_level->price_ > current_level->price_ && current_level->next_price_level_ != side_orders_by_price_head){
                current_level = current_level->next_price_level_;
            }

            new_price_level->next_price_level_ = current_level;
            new_price_level->prev_price_level_ = current_level->prev_price_level_;
            current_level->prev_price_level_->next_price_level_ = new_price_level;
            current_level->prev_price_level_ = new_price_level;

            if(new_price_level->price_ > side_orders_by_price_head->price_){
                bid_levels_head = new_price_level;
            }
        }else{
            while(new_price_level->price_ < current_level->price_ && current_level->next_price_level_ != side_orders_by_price_head){
                current_level = current_level->next_price_level_;
            }

            new_price_level->next_price_level_ = current_level;
            new_price_level->prev_price_level_ = current_level->prev_price_level_;
            current_level->prev_price_level_->next_price_level_ = new_price_level;
            current_level->prev_price_level_ = new_price_level;

            if(new_price_level->price_ < side_orders_by_price_head->price_){
                ask_levels_head = new_price_level;
            }
        }
    }

    auto MEOrderBook::getNextPriority(TickerId tickerId, Price price) noexcept -> Priority{
        const auto orders_at_price = getOrdersAtPrcie(price);
        if(!orders_at_price){
            // it is more optimal to return 1 as unsigned long, that 1 as int an then convert
            return 1lu;
        }
        // first -> prev = last :D
        return orders_at_price->first_order_->prev_order_->priority_ + 1;
    }

    auto MEOrderBook::addOrder(MEOrder* order) noexcept -> void {
        const auto price_level = getOrdersAtPrcie(order->price_);
        if(!price_level){
            //Add level
            order->next_order_ = order->prev_order_ = order;

            auto new_price_level = price_levels_pool_.allocate(order->side_, order->price_, order, nullptr, nullptr);
            addOrdersAtPrice(new_price_level);
        }else{
            // Append to price level
            auto first_order = price_level->first_order_;
            // Place order at the end
            first_order->prev_order_->next_order_ = order;
            order->prev_order_ = first_order->prev_order_;
            order->next_order_ = first_order;
            first_order->prev_order_ = order;
        }

        // Place order in hashmaps, Each client has hash map of orders
        client_orders_.at(order->clientId_).at(order->client_orderId_) = order;
    }

    auto MEOrderBook::add(ClientId clientId, OrderId client_orderId, TickerId tickerId, Side side, Price price, Qty qty) noexcept -> void {
        auto new_market_orderId = generateNewMarketOrderId();

        // this is ACCEPTED in FIX
        client_response_ = {ClientResponseType::ACCEPTED, clientId, tickerId, client_orderId,
                new_market_orderId, side, price, 0, qty};
        matching_engine_->sendClientResponse(&client_response_);

        //Check book
        const auto left_qty = checkForMatch(clientId, client_orderId, tickerId, side, price, qty, new_market_orderId);

        // If we have partiall fill
        if(LIKELY(left_qty)){
            const auto priority = getNextPriority(tickerId, price);
            auto order = order_pool_.allocate(tickerId, clientId, client_orderId, new_market_orderId, side, price, left_qty, priority, nullptr, nullptr);
            addOrder(order);

            market_update_ = {MarketUpdateType::ADD, new_market_orderId, tickerId, side, price, left_qty, priority};
            matching_engine_->sendMarketUpdate(&market_update_);
        }
    }

    auto MEOrderBook::cancel(ClientId clientId, OrderId client_orderId, TickerId tickerId) noexcept -> void{
        
    }
}
