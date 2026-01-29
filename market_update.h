#pragma once

#include <sstream>

#include "lf_queue.h"
#include "common_types.h"

using namespace Common;

namespace Exchange {
#pragma pack(push,1)
    enum class MarketUpdateType : uint8_t{
        INVALID = 0,
        ADD = 1,
        MODIFY = 2,
        CANCEL = 3,
        TRADE = 4
    };

    inline auto marketUpdateTypeToString(MarketUpdateType marketUpdateType) -> std::string{
        switch (marketUpdateType)
        {
            case MarketUpdateType::ADD:
                return "ADD";
                break;
            case MarketUpdateType::MODIFY:
                return "MODIFY";
                break;
            case MarketUpdateType::CANCEL:
                return "CANCEL";
                break;
            case MarketUpdateType::TRADE:
                return "TRADE";
                break;
            case MarketUpdateType::INVALID:
                return "INVALID";
                break;
        }

        return "UNKNOWN";
    }

    struct MEMarketUpdate {
        MarketUpdateType type_ = MarketUpdateType::INVALID;

        OrderId orderId_ = OrderId_INVALID;
        TickerId tickerId_ = TickerId_INVALID;
        Side side_ = Side::INVALID;
        Price price_ = Price_INVALID;
        Qty qty_ = Qty_INVALID;
        Priority priority_ = Priority_INVALID;

        auto toString() const -> std::string {
            std::stringstream stream;
            stream << "MEMarketUpdate"
            << " ["
            << "type:" << marketUpdateTypeToString(type_)
            << " oid: " << orderIdToString(orderId_)
            << " ticker: " << tickerIdToString(tickerId_)
            << " side: " << sideToString(side_)            
            << " price: " << priceToString(price_)
            << " qty: " << qtyToString(qty_)
            << " priority: " << priorityToString(priority_)
            << "]";

            return stream.str();
        }
    };
}