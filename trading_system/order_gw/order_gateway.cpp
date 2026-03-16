#include "order_gateway.h"

namespace TradingSystem
{   
    OrderGateway::OrderGateway(ClientId client_id, Exchange::ClientRequestLFQueue* outgoing_requests, Exchange::ClientResponseLFQueue* incoming_responses,
        std::string ip, const std::string iface, const int port): client_id_(client_id), outgoing_requests_(outgoing_requests), incoming_responses_(incoming_responses),
            ip_(ip), iface_(iface), port_(port_), logger_("trading_order_gateway" + std::to_string(client_id) + ".log"), tcp_socket_(logger_)
    {
        

    }

}