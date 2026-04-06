#pragma once

#include "common_types.h"

namespace TradingSystem{

    enum class RiskCheckResult : int8_t {
        INVALID = 0,
        ORDER_TOO_LARGE = 1,
        POSITION_TOO_LARGE = 2,
        LOSS_TOO_LARGE = 3,
        ALLOWED = 4
    };

    inline auto riskCheckResultToString(RiskCheckResult result) {
        switch (result) {
        case RiskCheckResult::INVALID:
            return "INVALID";
        case RiskCheckResult::ORDER_TOO_LARGE:
            return "ORDER_TOO_LARGE";
        case RiskCheckResult::POSITION_TOO_LARGE:
            return "POSITION_TOO_LARGE";
        case RiskCheckResult::LOSS_TOO_LARGE:
            return "LOSS_TOO_LARGE";
        case RiskCheckResult::ALLOWED:
            return "ALLOWED";
        }

        return "";
    }

}