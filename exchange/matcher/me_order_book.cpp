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
        bid_levels_head_ = nullptr;
        ask_levels_head_ = nullptr;

        for (auto &itr: client_orders_) {
            itr.fill(nullptr);
        }
    }

    // TODO: Try to do it brancheless
    auto MEOrderBook::addOrdersAtPrice(MEOrderAtPriceLevel* new_price_level) noexcept -> void{
        price_levels_.at(priceToIndex(new_price_level->price_)) = new_price_level;

        //Get head of double linked-list asks or bids
        auto side_orders_by_price_head = (new_price_level->side_ == Side::BUY) ? bid_levels_head_ : ask_levels_head_;

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
                bid_levels_head_ = new_price_level;
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
                ask_levels_head_ = new_price_level;
            }
        }
    }

    auto MEOrderBook::getNextPriority(TickerId tickerId, Price price) noexcept -> Priority{
        const auto orders_at_price = getPriceLevel(price);
        if(!orders_at_price){
            // it is more optimal to return 1 as unsigned long, that 1 as int an then convert
            return 1lu;
        }
        // first -> prev = last :D
        return orders_at_price->first_order_->prev_order_->priority_ + 1;
    }

    auto MEOrderBook::addOrder(MEOrder* order) noexcept -> void {
        const auto price_level = getPriceLevel(order->price_);
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

        //Check order book
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

    auto MEOrderBook::removePriceLevel(Side side, Price price) noexcept -> void{
        auto orders_head = (side == Side::BUY ? bid_levels_head_ : ask_levels_head_);
        const auto price_level = getPriceLevel(price);

        // Price level map is looped, next == current then it is last prive level
        // but we need to null head
        if(UNLIKELY(price_level->next_price_level_ == price_level)){
            orders_head = nullptr;
        } else {
            price_level->prev_price_level_->next_price_level_ = price_level->next_price_level_;
            price_level->next_price_level_->prev_price_level_ = price_level->prev_price_level_;
            //If we remove at head, change head
            if(orders_head == price_level){
                orders_head = price_level->next_price_level_;
            }
        }

        price_levels_.at(priceToIndex(price)) = nullptr;
        price_levels_pool_.deallocate(price_level);
    }

    auto MEOrderBook::removeOrder(MEOrder* order) noexcept -> void{
        auto price_level = getPriceLevel(order->price_);

        // If it points to itself = only one element, we can remove price level
        if(order->prev_order_ == order){
            removePriceLevel(order->side_, order->price_);
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

        client_orders_.at(order->clientId_).at(order->client_orderId_) = nullptr;
        order_pool_.deallocate(order);
    } 

    auto MEOrderBook::cancel(ClientId clientId, OrderId client_orderId, TickerId tickerId) noexcept -> void{
        // If id is bigger than our max index, cannot be cancelled
        auto is_cancelable = (clientId < client_orders_.size());

        MEOrder* order_to_cancel = nullptr;
        if(LIKELY(is_cancelable)){
            auto client_orders_itr = client_orders_.at(clientId);
            order_to_cancel = client_orders_itr.at(client_orderId);
            is_cancelable = (order_to_cancel != nullptr);

            if(UNLIKELY(!is_cancelable)){
                client_response_ = { ClientResponseType::CANCEL_REJECTED, clientId, tickerId, client_orderId, 
                    OrderId_INVALID, Side::INVALID, Price_INVALID, Qty_INVALID, Qty_INVALID };
            }else{
                client_response_ = { ClientResponseType::CANCELLED, clientId, tickerId, client_orderId, 
                    order_to_cancel->market_orderId_, order_to_cancel->side_, order_to_cancel->price_, Qty_INVALID, order_to_cancel->qty_ };

                market_update_ = { MarketUpdateType::CANCEL, client_orderId, tickerId, 
                    order_to_cancel->side_, order_to_cancel->price_, order_to_cancel->qty_, order_to_cancel->priority_};

                removeOrder(order_to_cancel);
                matching_engine_->sendMarketUpdate(&market_update_);
            }
            matching_engine_->sendClientResponse(&client_response_);
        }
    }

    auto MEOrderBook::match(TickerId tickerId, ClientId clientId, Side side, OrderId client_orderId, OrderId market_orderId, MEOrder* itr, Qty* left_qty) noexcept -> void{
        const auto order = itr; // matched order
        const auto order_qty = order->qty_; // matched order qty
        const auto fill_qty = std::min(*left_qty, order_qty); //fill with matched order qty or incoming order qty

        *left_qty -= fill_qty;
        order->qty_ -= fill_qty;

        //TODO: Should we add PARTIALL_FILL? or it will be done on propagating events
        //Incoming order
        client_response_ = {ClientResponseType::FILLED, clientId, tickerId, client_orderId,
                        market_orderId, side, itr->price_, fill_qty, *left_qty};
        matching_engine_->sendClientResponse(&client_response_);

        //Order Matched
        client_response_ = {ClientResponseType::FILLED, order->clientId_, order->tickerId_, order->client_orderId_,
                            order->market_orderId_, order->side_, itr->price_, fill_qty, order->qty_};
        matching_engine_->sendClientResponse(&client_response_);

        // Market Info
        market_update_ = {MarketUpdateType::TRADE, OrderId_INVALID, tickerId, side, itr->price_, fill_qty, Priority_INVALID};
        matching_engine_->sendMarketUpdate(&market_update_);

        if (!order->qty_) {
            market_update_ = {MarketUpdateType::CANCEL, order->market_orderId_, tickerId, order->side_,
                                order->price_, order_qty, Priority_INVALID};
            matching_engine_->sendMarketUpdate(&market_update_);

            removeOrder(order);
        } else {
            market_update_ = {MarketUpdateType::MODIFY, order->market_orderId_, tickerId, order->side_,
                                order->price_, order->qty_, order->priority_};
            matching_engine_->sendMarketUpdate(&market_update_);
        }
    }

    auto MEOrderBook::checkForMatch(ClientId clientId, OrderId client_orderId, TickerId tickerId, Side side, Price price, Qty qty, OrderId market_orderId) noexcept -> Qty{
        auto left_qty = qty;

        if(side == Side::BUY){
            while(left_qty && ask_levels_head_){
                const auto ask_orders_itr = ask_levels_head_->first_order_;
                if(LIKELY(price < ask_orders_itr->price_)){
                    //There is no price that can fill this BUY order
                    break;
                }
                match(tickerId, clientId, side, client_orderId, market_orderId, ask_orders_itr, &left_qty);
            }
        }

        if(side == Side::SELL){
            while(left_qty && bid_levels_head_){
                const auto bid_orders_itr = bid_levels_head_->first_order_;
                if(LIKELY(price > bid_orders_itr->price_)){
                    //There is no price that can fill this ASK order
                    break;
                }
                match(tickerId, clientId, side, client_orderId, market_orderId, bid_orders_itr, &left_qty);
            }
        }

        return left_qty;
    }
}
