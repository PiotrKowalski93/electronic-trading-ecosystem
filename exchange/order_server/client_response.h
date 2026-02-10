#pragma once

#include <sstream>

#include "common_types.h"
#include "lf_queue.h"

using namespace Common;

namespace Exchange {
    //pack(push,1) -> we can set it on non-hot-path
    // CPU side access can be slower
    // We gaint simple wire format, less bytes
#pragma pack(push,1)
    enum class ClientResponseType : uint8_t{
        INVALID = 0,
        ACCEPTED = 1,
        CANCELLED = 2,
        FILLED = 3,
        PARTIAL_FILL = 4,
        CANCEL_REJECTED = 5
    };

    inline auto clientResponseTypeToString(ClientResponseType responseType) -> std::string {
        switch (responseType)
        {
            case ClientResponseType::ACCEPTED:
                return "ACCEPTED";
                break;
            case ClientResponseType::CANCELLED:
                return "CANCELLED";
                break;
            case ClientResponseType::FILLED:
                return "FILLED";
                break;
            case ClientResponseType::PARTIAL_FILL:
                return "PARTIAL_FILL";
                break;
            case ClientResponseType::CANCEL_REJECTED:
                return "CANCEL_REJECTED";
                break;
            case ClientResponseType::INVALID:
                return "INVALID";
                break;
        }
        return "UNKNOWN";
    }

    struct MEClientResponse {
        ClientResponseType type_ = ClientResponseType::INVALID;

        ClientId clientId_ = ClientId_INVALID;
        TickerId tickerId_ = TickerId_INVALID;
        OrderId client_order_id_ = OrderId_INVALID;
        OrderId marker_order_id = OrderId_INVALID;
        Side side_ = Side::INVALID;
        Price price_ = Price_INVALID;
        Qty exec_qty_ = Qty_INVALID;
        Qty left_qty_ = Qty_INVALID;

        auto toString() const {
            std::stringstream stream;
            stream << "MEClientResponse"
            << " ["
            << "type:" << clientResponseTypeToString(type_)
            << " client: " << clientIdToString(clientId_)
            << " ticker: " << tickerIdToString(tickerId_)
            << " client_oid: " << orderIdToString(client_order_id_)
            << " market_oid: " << orderIdToString(marker_order_id)
            << " side: " << sideToString(side_)            
            << " price: " << priceToString(price_)
            << " executed_qty: " << qtyToString(exec_qty_)
            << " left_qty: " << qtyToString(left_qty_)
            << "]";

            return stream.str();
        }

    };

    struct OMClientResponse {
        size_t seq_num = 0;
        MEClientResponse me_client_response;

        auto toString() const -> std::string{
            std::stringstream ss;
            ss << "OMClientResponse: " 
                << "[ "
                << "sequence number: " << seq_num
                << me_client_response.toString()
                << "]";
            return ss.str();
        }

    };

#pragma pack(pop)

    typedef LFQueue<MEClientResponse> ClientResponseLFQueue;
}