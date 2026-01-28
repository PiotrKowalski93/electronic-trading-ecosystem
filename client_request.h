#pragma once

#include <sstream>

#include "common_types.h"
#include "lf_queue.h"

using namespace Common;

namespace Exchange {
    // Aligment for structures -> 1 bajt
    #pragma pack(push,1)

    enum class ClientRequestType : uint8_t {
        INVALID = 0,
        NEW = 1,
        CANCEL = 2
    };

    inline auto clientRequestTypeToString(ClientRequestType clientRequestType) -> std::string {
        switch (clientRequestType) {
            case ClientRequestType::INVALID:
                return "INVALID";
                break;
            case ClientRequestType::NEW:
                return "NEW";
                break;
            case ClientRequestType::CANCEL:
                return "CANCEL";
                break;
            default:
                break;
        }
        return "UNKNOWN";
    }

    struct MEClientRequest {
        ClientRequestType requestType_ = ClientRequestType::INVALID;

        ClientId clientId_ = ClientId_INVALID;        
        TickerId tickerId_ = TickerId_INVALID;
        OrderId orderId_ = OrderId_INVALID;
        Price price_ = Price_INVALID;
        Qty qty_ = Qty_INVALID;
        Priority priority = Priority_INVALID;
        Side side_ = Side::INVALID;

        auto toString() const {
            std::stringstream stream;
            stream << "MEClientRequest"
            << " ["
            << "type:" << clientRequestTypeToString(requestType_)
            << " client: " << clientIdToString(clientId_)
            << " ticker: " << tickerIdToString(tickerId_)
            << " oid: " << orderIdToString(orderId_)
            << " side: " << sideToString(side_)
            << " qty: " << qtyToString(qty_)
            << " price: " << priceToString(price_)
            << "]";

            return stream.str();
        }
    };

#pragma pack(pop)

    typedef LFQueue<MEClientRequest> ClientRequestLFQueue;
}