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

auto prepare_real_requests(std::vector<Exchange::MEClientRequest>* client_requests)
{
    using namespace Exchange;

    constexpr size_t TOTAL = 1'000'000;
    constexpr double NEW_RATIO = 0.70;
    constexpr double CANCEL_RATIO = 0.20;
    constexpr double MODIFY_RATIO = 0.10;

    constexpr int NUM_TICKERS = 5;
    constexpr Price BASE_MID_PRICE = 10000; // np 100.00 przy tick=1
    constexpr Price TICK_SIZE = 1;

    client_requests->clear();
    client_requests->reserve(TOTAL);

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

    std::vector<OrderId> active_orders;
    active_orders.reserve(TOTAL);

    OrderId next_order_id = 1;

    for (size_t i = 0; i < TOTAL; ++i)
    {
        double p = prob_dist(rng);

        MEClientRequest req{};

        // --- NEW ---
        if (p < NEW_RATIO || active_orders.empty())
        {
            req.requestType_ = ClientRequestType::NEW;
            req.orderId_ = next_order_id++;
            req.clientId_ = static_cast<ClientId>(req.orderId_ % 1000);
            req.tickerId_ = static_cast<TickerId>(ticker_dist(rng));

            req.side_ = side_dist(rng) == 0 ? Side::BUY : Side::SELL;

            // cena wokół mid
            Price delta = static_cast<Price>(std::round(price_dist(rng)));
            req.price_ = BASE_MID_PRICE + delta * TICK_SIZE;

            req.qty_ = power_law_qty();

            active_orders.push_back(req.orderId_);
        }
        // --- CANCEL ---
        else if (p < NEW_RATIO + CANCEL_RATIO)
        {
            req.requestType_ = ClientRequestType::CANCEL;

            std::uniform_int_distribution<size_t> cancel_dist(0, active_orders.size() - 1);
            size_t idx = cancel_dist(rng);

            req.orderId_ = active_orders[idx];
            req.clientId_ = static_cast<ClientId>(req.orderId_ % 1000);
            req.tickerId_ = static_cast<TickerId>(0); // engine powinien lookupować

            req.side_ = Side::INVALID;
            req.price_ = Price_INVALID;
            req.qty_ = Qty_INVALID;

            // usuń z active
            active_orders[idx] = active_orders.back();
            active_orders.pop_back();
        }
        // --- MODIFY ---
        else
        {
            req.requestType_ = ClientRequestType::NEW; // traktujemy jako replace

            std::uniform_int_distribution<size_t> mod_dist(0, active_orders.size() - 1);
            size_t idx = mod_dist(rng);

            req.orderId_ = active_orders[idx];
            req.clientId_ = static_cast<ClientId>(req.orderId_ % 1000);
            req.tickerId_ = static_cast<TickerId>(ticker_dist(rng));

            req.side_ = side_dist(rng) == 0 ? Side::BUY : Side::SELL;

            Price delta = static_cast<Price>(std::round(price_dist(rng)));
            req.price_ = BASE_MID_PRICE + delta * TICK_SIZE;

            req.qty_ = power_law_qty();
        }

        client_requests->push_back(req);
    }
}


int main(){
    // Prepare benchmark
    const size_t NUMBER_OF_ORDERS = 1000000;    // 1 000 000
    const size_t WARMUP_COUNT = 100000;         // 100 000

    Exchange::MatchingEngine* engine = new Exchange::MatchingEngine(nullptr, nullptr, nullptr);

    std::vector<Exchange::MEClientRequest> client_requests;
    client_requests.reserve(NUMBER_OF_ORDERS);

    std::vector<uint64_t> latencies;
    latencies.reserve(NUMBER_OF_ORDERS);

    // Prepare requests
    std::cout << "Prepare requests" << std::endl;
    prepare_real_requests(&client_requests);

    // Warmup
    std::cout << "Warmup" << std::endl;
    for (size_t i = 0; i < 100'000; ++i)
        engine->processClientRequest(&client_requests[i]);

    // Benchmark
    std::cout << "Benchmark" << std::endl;
    for (size_t i = 100'000; i < client_requests.size(); ++i){
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
}