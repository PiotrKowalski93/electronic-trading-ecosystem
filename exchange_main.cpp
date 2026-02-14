#include <csignal>

#include "exchange/matcher/matching_engine.h"
#include "exchange/order_server/order_server.h"

Common::Logger* logger = nullptr;
Exchange::MatchingEngine* matching_engine = nullptr;
Exchange::OrderServer *order_server = nullptr;

void signal_handler(int){
    using namespace std::literals::chrono_literals;
    std::this_thread::sleep_for(10s);

    delete logger;
    logger = nullptr;

    delete matching_engine;
    matching_engine = nullptr;

    std::this_thread::sleep_for(10s);
    exit(EXIT_SUCCESS);
}

int main (){
    std::string log_filename = "exchange_main.log";

    logger = new Common::Logger(log_filename);

    const int sleep_time = 100 * 1000;

    Exchange::ClientRequestLFQueue client_requests(ME_MAX_CLIENT_UPDATES);
    Exchange::ClientResponseLFQueue client_responses(ME_MAX_CLIENT_UPDATES);
    Exchange::MarketDataLFQueue market_updates(ME_MAX_MARKET_UPDATES);

    std::string time_str;

    // ---------------- Matching Engine ----------------
    logger->log("%:% %() % Starting MatchingEngine...\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str));

    matching_engine = new Exchange::MatchingEngine(&client_requests, &client_responses, &market_updates);
    matching_engine->start();

    // ---------------- Order Server ----------------
    const std::string order_gw_iface = "lo";
    const int order_gw_port = 12345;

    logger->log("%:% %() % Starting Order Server...\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str));
    order_server = new Exchange::OrderServer(&client_requests, &client_responses, order_gw_iface, order_gw_port);
    order_server->start();

    while(true){
        logger->log("%:% %() % Sleeping...\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str));   
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(2s); // Less?
    }
}