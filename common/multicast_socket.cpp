#include "multicast_socket.h"

namespace Common {

    auto MulticastSocket::init(const std::string &ip, const std::string &iface, int port, bool is_listening) -> int {
        socket_fd_ = createSocket(logger_, ip, iface, port, true, false, is_listening, 1, false);
        return socket_fd_;
    }

    bool MulticastSocket::join(const std::string &ip) {
        return Common::join(socket_fd_, ip);
    }

}