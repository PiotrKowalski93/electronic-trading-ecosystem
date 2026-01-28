#include <cstdint>
#include <limits>
#include <iostream>

#include "macros.h"

namespace Common {

    constexpr auto _INVALID = "INVALID";

    typedef uint64_t OrderId;
    constexpr auto OrderId_INVALID = std::numeric_limits<OrderId>::max();

    typedef uint32_t TickerId;
    constexpr auto TickerId_INVALID = std::numeric_limits<TickerId>::max();

    typedef uint32_t ClientId;
    constexpr auto ClientId_INVALID = std::numeric_limits<TickerId>::max();

    typedef uint64_t Price;
    constexpr auto Price_INVALID = std::numeric_limits<Price>::max();

    typedef uint32_t Qty;
    constexpr auto Qty_INVALID = std::numeric_limits<Qty>::max();

    typedef uint64_t Priority;
    constexpr auto Priority_INVALID = std::numeric_limits<Priority>::max();

    enum class Side : uint8_t {
        INVALID = 0,
        BUY = 1,
        SELL = 2
    };

    inline auto orderIdToString(OrderId orderId) -> std::string {
        if(UNLIKELY(orderId == OrderId_INVALID)) return _INVALID; 
        return std::to_string(orderId);
    }

    inline auto tickerIdToString(TickerId tickerId) -> std::string {
        if(UNLIKELY(tickerId == TickerId_INVALID)) return _INVALID;
        return std::to_string(tickerId);
    }

    inline auto clientIdToString(ClientId clientId) -> std::string {
        if(UNLIKELY(clientId == ClientId_INVALID)) return _INVALID;
        return std::to_string(clientId);
    }

    inline auto priceToString(Price price) -> std::string {
        if(UNLIKELY(price == Price_INVALID)) return _INVALID;
        return std::to_string(price);
    }

    inline auto qtyToString(Qty qty) -> std::string {
        if(UNLIKELY(qty == Qty_INVALID)) return _INVALID;
        return std::to_string(qty);
    }

    inline auto priorityToString(Priority priority) -> std::string {
        if(UNLIKELY(priority == Priority_INVALID)) return _INVALID;
        return std::to_string(priority);
    }

    inline auto sideToString(Side side) -> std::string {
        switch (side) {
            case Side::INVALID:
                return _INVALID;
                break;
            case Side::BUY:
                return "BUY";
                break;
            case Side::SELL:
                return "SELL";
                break;
            default:
                break;
        }
        return "UNKNOWN";
    }
}