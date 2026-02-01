#include "matching_engine.h"

namespace Exchange
{
    std::string maching_engine_log_filename = "maching_engine.log";

    MatchingEngine::MatchingEngine(ClientRequestLFQueue* client_requests, 
        ClientResponseLFQueue* client_responses, 
        MarketDataLFQueue* market_updates)
        : incoming_requests_(client_requests), outgoing_responses_(client_responses), 
        outgoing_market_updates_(market_updates), logger_(maching_engine_log_filename)
        {
            //TODO: Init OrderBook field
        };

    MatchingEngine::~MatchingEngine() {
        is_running_ = false;

        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(1s);

        incoming_requests_ = nullptr;
        outgoing_responses_ = nullptr;
        outgoing_market_updates_ = nullptr;

        //TODO: DeInit OrderBook field
    }

    auto MatchingEngine::processClientRequest(const MEClientRequest* client_request) noexcept -> void {
        //TODO: Implement when we have OrderBook
        //this method passes order to correct bucket (ticker_id)
    }

    auto MatchingEngine::sendClientResponse(const MEClientResponse* client_response) noexcept -> void {
        logger_.log("%:% %() % Sending response: %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), client_response->toString());
        auto next_write = outgoing_responses_->getNextToWriteTo();
        *next_write = std::move(*client_response);
        outgoing_responses_->updateNextToWriteTo();
    }

    auto MatchingEngine::start() -> void {
        is_running_ = true;

        ASSERT(Common::createAndStartThread(-1, "Exchange/MatchingEngine", [this](){ run(); }) != nullptr,
                "Failded to start MatchingEngine thread.");        
    }

    auto MatchingEngine::stop() -> void {
        is_running_ = false;
    }

    auto MatchingEngine::run() -> void {
        logger_.log("%:% %() %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));

        while(is_running_){
            const auto me_client_request = incoming_requests_->getNextToRead();

            if(LIKELY(me_client_request)){
                logger_.log("%:% %() % Processing: %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), me_client_request->toString());
                processClientRequest(me_client_request);
                incoming_requests_->updateReadIndex();
            }
        }
    }
};
