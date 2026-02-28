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

    auto SnapshotSynthesizer::publishSnapshot() -> void{
        size_t snapshot_size = 0;

        // First msg in multicast SNAPSHOT_START
        const MDPMarketUpdate snapshot_start_msg {
            snapshot_size++,
            {
                MarketUpdateType::SNAPSHOT_START,
                last_inc_seq_num_
            }
        };

        logger_.log("%:% %() % %\n", __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&time_str_), snapshot_start_msg.toString());

        snapshot_socket_.send(&snapshot_start_msg, sizeof(MDPMarketUpdate));

        // Iterate throught tickers and their orders, send CLEAR
        for(size_t tickerId = 0; tickerId < orders_per_ticker_.size(); ++tickerId){
            const auto &orders = orders_per_ticker_.at(tickerId);

            MEMarketUpdate me_market_update;
            me_market_update.type_ = MarketUpdateType::CLEAR;
            me_market_update.tickerId_ = tickerId;

            const MDPMarketUpdate clear_market_update{snapshot_size++, me_market_update};
            logger_.log("%:% %() % %\n", __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&time_str_), clear_market_update.toString());

            snapshot_socket_.send(&clear_market_update, sizeof(MDPMarketUpdate));
            
            // Send Orders
            for(const auto order : orders){
                if(order){
                    const MDPMarketUpdate order_market_update {snapshot_size++, *order };
                    logger_.log("%:% %() % %\n", __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&time_str_), order_market_update.toString());

                    snapshot_socket_.send(&order_market_update, sizeof(MDPMarketUpdate));
                    snapshot_socket_.sendAndRecv();
                }
            }
        }

        const MDPMarketUpdate snapshot_end_msg {
            snapshot_size++,
            {
                MarketUpdateType::SNAPSHOT_END,
                last_inc_seq_num_
            }
        };

        logger_.log("%:% %() % %\n", __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&time_str_), snapshot_end_msg.toString());

        snapshot_socket_.send(&snapshot_end_msg, sizeof(MDPMarketUpdate));
        snapshot_socket_.sendAndRecv();

        logger_.log("%:% %() % Published snapshot: %\n", __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&time_str_), snapshot_end_msg.toString());
    }

    auto SnapshotSynthesizer::run() noexcept -> void{
        logger_.log("%:% %() %\n", __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&time_str_));

        // Main loop for SnapshotSynthesizer
        while(is_running){
            for(auto marker_update = snapshot_marker_updates_->getNextToRead(); 
                    snapshot_marker_updates_->size() && marker_update; 
                    marker_update = snapshot_marker_updates_->getNextToRead()){
                logger_.log("%:% %() % Processing market update: %\n", __FILE__, __LINE__, __FUNCTION__, getCurrentTimeStr(&time_str_), marker_update->toString());

                addToSnapshot(marker_update);
                snapshot_marker_updates_->updateReadIndex();
            }

            if(getCurrentNanos() - last_snapshot_time_ > 60 * NANOS_TO_SECS){
                last_snapshot_time_ = getCurrentNanos();
                publishSnapshot();
            }
        }
    }
    
}