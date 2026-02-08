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
            for(size_t i = 0; i < ticker_order_book_.size(); ++i) {
                ticker_order_book_[i] = new MEOrderBook(i, this, &logger_);
            }
        };

    MatchingEngine::~MatchingEngine() {
        is_running_ = false;

        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(1s);

        incoming_requests_ = nullptr;
        outgoing_responses_ = nullptr;
        outgoing_market_updates_ = nullptr;

        for(auto& order_book : ticker_order_book_) {
            delete order_book;
            order_book = nullptr;
        }
    }

    auto MatchingEngine::processClientRequest(const MEClientRequest* client_request) noexcept -> void {
        auto order_book_ = ticker_order_book_[client_request->tickerId_];

        switch (client_request->requestType_)
        {
            case ClientRequestType::NEW:
                order_book_->add(client_request->clientId_, client_request->orderId_, client_request->tickerId_,
                    client_request->side_, client_request->price_, client_request->qty_);
                break;
            case ClientRequestType::CANCEL:
                order_book_->cancel(client_request->clientId_, client_request->orderId_, client_request->tickerId_);
                break;
            default:
                FATAL("Recieved invalid requestType_: " + clientRequestTypeToString(client_request->requestType_));
                break;
        }
    }

    auto MatchingEngine::sendClientResponse(const MEClientResponse* client_response) noexcept -> void {
        logger_.log("%:% %() % Sending response: %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), client_response->toString());
        auto next_write = outgoing_responses_->getNextToWriteTo();
        *next_write = std::move(*client_response);
        outgoing_responses_->updateNextToWriteTo();
    }

    auto MatchingEngine::sendMarketUpdate(const MEMarketUpdate* market_update) noexcept -> void{
        logger_.log("%:% %() % Sending market update: %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), market_update->toString());
        auto next_write = outgoing_market_updates_->getNextToWriteTo();
        *next_write = std::move(*market_update);
        outgoing_market_updates_->updateNextToWriteTo();
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
