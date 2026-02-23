#include <vector>
#include <random>
#include <cstdint>
#include <cmath>
#include <time.h>
#include <algorithm>
#include <x86intrin.h>

#include "logging.h"
#include "matching_engine.h"

inline uint64_t rdtsc() {
    return __rdtsc();
}

auto prepare_real_requests(std::vector<Exchange::MEClientRequest>* client_requests, size_t n)
{
    using namespace Exchange;

    constexpr int NUM_TICKERS = 5;
    constexpr Price BASE_MID_PRICE = 10000; // np 100.00 przy tick=1
    constexpr Price TICK_SIZE = 1;

    client_requests->clear();
    client_requests->reserve(n);

    std::mt19937_64 rng(42);

    std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
    std::uniform_int_distribution<int> side_dist(0, 1);
    std::uniform_int_distribution<int> ticker_dist(0, NUM_TICKERS - 1);
    std::normal_distribution<double> price_dist(0.0, 5.0);

    // power law qty ~ 1/x^alpha
    auto power_law_qty = [&]() -> Qty {
        constexpr double alpha = 1.5;
        constexpr double min_qty = 1.0;
        constexpr double max_qty = 1000.0;

        double u = prob_dist(rng);
        double qty = min_qty * std::pow(1 - u, -1.0 / (alpha - 1.0));

        if (qty > max_qty)
            qty = max_qty;

        return static_cast<Qty>(qty);
    };

    OrderId next_order_id = 1;

    for (size_t i = 0; i < n; ++i)
    {
        double p = prob_dist(rng);

        MEClientRequest req{};

        req.requestType_ = ClientRequestType::NEW;
        req.orderId_ = next_order_id++;
        req.clientId_ = static_cast<ClientId>(req.orderId_ % 1000);
        req.tickerId_ = static_cast<TickerId>(ticker_dist(rng));

        req.side_ = side_dist(rng) == 0 ? Side::BUY : Side::SELL;

        Price delta = static_cast<Price>(std::round(price_dist(rng)));
        req.price_ = BASE_MID_PRICE + delta * TICK_SIZE;

        req.qty_ = power_law_qty();

        client_requests->push_back(req);

        std::cout << req.toString() << std::endl;
    }
}


int main(){
    // Prepare benchmark
    const size_t NUMBER_OF_ORDERS = 10; 
    const size_t WARMUP_COUNT = 2;
    //const size_t NUMBER_OF_ORDERS = 1'000'000;    // 1 000 000
    //const size_t WARMUP_COUNT = 100'000;         // 100 000

    // Prepare dependencies
    Exchange::ClientRequestLFQueue* client_requests_q = new Exchange::ClientRequestLFQueue(NUMBER_OF_ORDERS);
    Exchange::ClientResponseLFQueue* client_responses_q = new Exchange::ClientResponseLFQueue(NUMBER_OF_ORDERS); 
    Exchange::MarketDataLFQueue* market_updates_q = new Exchange::MarketDataLFQueue(NUMBER_OF_ORDERS);

    Exchange::MatchingEngine* engine = new Exchange::MatchingEngine(client_requests_q, client_responses_q, market_updates_q);

    std::vector<Exchange::MEClientRequest> client_requests;
    client_requests.reserve(NUMBER_OF_ORDERS);

    std::vector<uint64_t> latencies;
    latencies.reserve(NUMBER_OF_ORDERS);

    // Prepare requests
    std::cout << "Prepare requests" << std::endl;
    prepare_real_requests(&client_requests, NUMBER_OF_ORDERS);

    // Warmup
    std::cout << "Warmup" << std::endl;
    for (size_t i = 0; i < WARMUP_COUNT; ++i){
        //std::cout << i << ",";
        engine->processClientRequest(&client_requests[i]);
    }

    // Benchmark
    std::cout << "Benchmark" << std::endl;
    for (size_t i = WARMUP_COUNT; i < client_requests.size(); ++i){
        // processor time stamp - The processor time stamp records the number of clock cycles since the last reset.
        auto start = rdtsc();
        engine->processClientRequest(&client_requests[i]);
        auto end = rdtsc();

        latencies.push_back(end - start);
        std::cout << end - start << std::endl;
    }

    // Save results
    std::sort(latencies.begin(), latencies.end());
    //auto p50  = latencies[N * 0.50];
    //auto p99  = latencies[N * 0.99];
    //auto p999 = latencies[N * 0.999];
    //auto max  = latencies.back();
    for(uint64_t l : latencies){
        std::cout << l << ",";
    }
}