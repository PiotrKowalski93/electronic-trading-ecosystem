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
        bids_levels_ = nullptr;
        asks_levels_ = nullptr;

        for (auto &itr: client_orders_) {
            itr.fill(nullptr);
        }
    }

    auto MEOrderBook::getNextPriority(TickerId tickerId, Price price) noexcept -> Priority{
        const auto orders_at_price = getOrdersAtPrcie(price);
        if(!orders_at_price){
            // it is more optimal to return 1 as unsigned long, that 1 as int an then convert
            return 1lu;
        }

        // first -> prev = last :D
        return orders_at_price->first_Order_->prev_order_->priority_ +1;
    }

    auto MEOrderBook::addOrder(MEOrder* order) noexcept -> void {
        const auto price_level = getOrdersAtPrcie(order->price_);
        if(!price_level){
            //Add level
            order->next_order_ = order->prev_order_ = order;

            auto new_price_level = price_levels_pool_.allocate(order->side_, order->price_, order, nullptr, nullptr);
            addOrderAtPrice(new_price_level);
        }

        // Append to price level
        
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
}
