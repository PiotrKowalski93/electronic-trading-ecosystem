#pragma once

#include "lf_queue.h"
#include "macros.h"
#include "thread_utils.h"

#include "order_server/client_request.h"
#include "order_server/client_response.h"
#include "market_data/market_update.h"

#include "logging.h"

#include "me_order_book.h"

namespace Exchange
{
    class MatchingEngine final {
        public:
            MatchingEngine(ClientRequestLFQueue* client_requests, ClientResponseLFQueue* client_responses, 
                MarketDataLFQueue* market_updates);
            ~MatchingEngine();

            auto start() -> void;
            auto run() -> void;
            auto stop() -> void;

            MatchingEngine() = delete;
            MatchingEngine(const MatchingEngine&) = delete;
            MatchingEngine(const MatchingEngine&&) = delete;
            MatchingEngine& operator=(const MatchingEngine&) = delete;
            MatchingEngine& operator=(const MatchingEngine&&) = delete;

            auto sendClientResponse(const MEClientResponse* client_response) noexcept -> void;
            auto sendMarketUpdate(const MEMarketUpdate* market_update) noexcept -> void;

        private:
            OrderBookHashMap ticker_order_book_;
            ClientRequestLFQueue *incoming_requests_ = nullptr;
            ClientResponseLFQueue *outgoing_responses_ = nullptr;
            MarketDataLFQueue *outgoing_market_updates_ = nullptr;

            volatile bool is_running_ = false;

            std::string time_str_;
            Logger logger_;

            auto processClientRequest(const MEClientRequest* client_request) noexcept -> void;
    };
}