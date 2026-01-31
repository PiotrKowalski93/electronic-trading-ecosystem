#pragma once

#include "matching_engine.h"

namespace Exchange
{
    MatchingEngine::MatchingEngine(ClientRequestLFQueue *client_requests, 
        ClientResponseLFQueue *client_responses, 
        MarketDataLFQueue *market_updates)
        : incoming_requests_(client_requests), outgoing_responses_(client_responses), 
        outgoing_market_updates_(market_updates), logger_("maching_engine.log")
        {};
};
