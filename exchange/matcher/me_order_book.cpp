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

    auto MEOrderBook::add(ClientId clientId, OrderId client_orderId, TickerId tickerId, Side side, Price price, Qty qty) noexcept -> void {
        auto new_market_orderId = generateNewMarketOrderId();

        // this is ACCEPTED in FIX
        client_response_ = {ClientResponseType::ACCEPTED, clientId, tickerId, client_orderId,
                new_market_orderId, side, price, 0, qty};
        matching_engine_->sendClientResponse(&client_response_);

        //Check book
        const auto left_qty = checkForMatch(clientId, client_orderId, tickerId, side, price, qty, new_market_orderId);

    }
}
