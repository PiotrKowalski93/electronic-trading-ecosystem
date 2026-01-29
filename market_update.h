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
}