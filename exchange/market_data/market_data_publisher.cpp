#include <cstring>

#include "market_data_publisher.h"

namespace Exchange{
    std::string market_data_publisher_log_filename = "market_data_publisher.log";

    MarketDataPublisher::MarketDataPublisher(MarketDataLFQueue* outgoing_md_updates, 
        const std::string &iface,
        const std::string &snapshot_ip, 
        const int snapshot_port,
        const std::string &incremental_ip, 
        const int incremental_port) 
        : outgoing_md_updates_(outgoing_md_updates), 
            snapshot_updates_(ME_MAX_MARKET_UPDATES), 
            is_running_(false),
            logger_(market_data_publisher_log_filename),
            incremental_socket_(logger_)
    {
        ASSERT(incremental_socket_.init(incremental_ip, iface, incremental_port, /*is_listening*/ false) >= 0, 
            "Unable to create incremental mcast socket. error:" + std::string(std::strerror(errno)));
        snapshot_synthesizer_ = new SnapshotSynthesizer(&snapshot_updates_, iface, snapshot_ip, snapshot_port);
    }

    MarketDataPublisher::~MarketDataPublisher(){
        stop();

        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(5s);

        delete snapshot_synthesizer_;
        snapshot_synthesizer_ = nullptr;
        // TODO: why not destroing other *?
    }

    auto MarketDataPublisher::run() noexcept -> void{
        logger_.log("%:% %() %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));

        //Main loop
        while(is_running_){
            //Batch sending from queue
            for(auto market_update = outgoing_md_updates_->getNextToRead(); outgoing_md_updates_->size() && market_update; market_update = outgoing_md_updates_->getNextToRead()){
                logger_.log("%:% %() % Sending seq:% %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), next_inc_seq_num_, market_update->toString().c_str());

                incremental_socket_.send(&next_inc_seq_num_, sizeof(next_inc_seq_num_));
                incremental_socket_.send(market_update, sizeof(MEMarketUpdate));

                //Move index for reads after sending
                outgoing_md_updates_->updateReadIndex();
                
                // auto next_write = snapshot_md_updates_.getNextToWriteTo();
                // next_write->seq_num_ = next_inc_seq_num_;
                // next_write->me_market_update_ = *market_update;
                // snapshot_md_updates_.updateWriteIndex();

                ++next_inc_seq_num_;
            }

            //After "sending batch" send it to clients and read incoming
            incremental_socket_.sendAndRecv();
        }
    }

    auto MarketDataPublisher::start() -> void {
        is_running_ = true;
        ASSERT(Common::createAndStartThread(-1, "Exchange/MarketDataPublisher", [this]() { run(); }) != nullptr, "Failed to start MarketData thread.");
        snapshot_synthesizer_->start();
    }

    auto MarketDataPublisher::stop() -> void{
        is_running_ = false;
        snapshot_synthesizer_->stop();
    }
}