#include "snapshot_synthesizer.h"

namespace Exchange{
    std::string snapshot_synthesizer_log_filename = "snapshot_synthesizer.log";

    SnapshotSynthesizer::SnapshotSynthesizer(MDPMarketDataLFQueue* market_updates, 
        const std::string &iface,
        const std::string &snapshot_ip,
        int snapshot_port) : snapshot_marker_updates_(market_updates),
                            logger_(snapshot_synthesizer_log_filename),
                            snapshot_socket_(logger_),
                            order_pool_(ME_MAX_ORDER_IDS){
        //TODO: Log error from errno
        ASSERT(snapshot_socket_.init(snapshot_ip, iface, snapshot_port, /*is_listening*/ false) >= 0, 
            "Unable to create snapshot mcast socket.");
    }

    SnapshotSynthesizer::~SnapshotSynthesizer(){
        stop();
    }

    auto SnapshotSynthesizer::start() -> void {
        ASSERT(Common::createAndStartThread(-1, "Exchange/SnapshotSynthesizer", [this](){run();}) != nullptr, 
            "Failed to start SnapshotSynthesizer thread.");
    }

    auto SnapshotSynthesizer::stop() -> void {
        is_running = false;
    }

    auto SnapshotSynthesizer::addToSnapshot(const MDPMarketUpdate* market_update) -> void{
        const auto &me_market_update = market_update->me_market_update;
        auto* orders = &orders_per_ticker_.at(me_market_update.tickerId_);

        switch (me_market_update.type_)
        {
            case MarketUpdateType::ADD: {
                auto order = orders->at(me_market_update.orderId_);
                ASSERT(order == nullptr, "Recieved: + " + me_market_update.toString() + " but order already exists:" + (order ? order->toString() : ""));
                orders->at(me_market_update.orderId_) = order_pool_.allocate(me_market_update);
                break;
            }
            case MarketUpdateType::MODIFY: {
                auto order = orders->at(me_market_update.orderId_);

                ASSERT(order != nullptr, "Recieved: + " + me_market_update.toString() + " but order does not exist.");
                ASSERT(order->orderId_ == me_market_update.orderId_, "Market update order_id does not match exisitng one");
                ASSERT(order->side_ == me_market_update.side_, "Market update side does not match order side");

                order->qty_ = me_market_update.qty_;
                order->price_ = me_market_update.price_;
                break;
            }
            case MarketUpdateType::CANCEL: {
                auto order = orders->at(me_market_update.orderId_);

                ASSERT(order != nullptr, "Recieved: + " + me_market_update.toString() + " but order does not exist.");
                ASSERT(order->orderId_ == me_market_update.orderId_, "Market update order_id does not match exisitng one");
                ASSERT(order->side_ == me_market_update.side_, "Market update side does not match order side");

                order_pool_.deallocate(order);
                orders->at(me_market_update.orderId_) = nullptr;
                break;
            }
            case MarketUpdateType::CLEAR:
            case MarketUpdateType::TRADE:
            case MarketUpdateType::INVALID:
            case MarketUpdateType::SNAPSHOT_START:
            case MarketUpdateType::SNAPSHOT_END:
                break;
        }

        ASSERT(market_update->seq_num == last_inc_seq_num_ + 1, "Expected incremental seq number to increase.");
        last_inc_seq_num_ = market_update->seq_num;
    }
}