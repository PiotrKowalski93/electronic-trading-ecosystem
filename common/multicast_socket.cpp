#include "multicast_socket.h"

namespace Common {

    auto MulticastSocket::init(const std::string &ip, const std::string &iface, int port, bool is_listening) -> int {
        socket_fd_ = createSocket(logger_, ip, iface, port, true, false, is_listening, 1, false);
        return socket_fd_;
    }

    bool MulticastSocket::join(const std::string &ip) {
        return Common::join(socket_fd_, ip);
    }

    auto MulticastSocket::leave(const std::string &, int) -> void {
        close(socket_fd_);
        socket_fd_ = -1;
    }

    auto MulticastSocket::sendAndRecv() noexcept -> bool {
        const ssize_t n_rcv = recv(socket_fd_, inbound_data_.data() + next_rcv_valid_index_, MulticastBufferSize - next_rcv_valid_index_, MSG_DONTWAIT);
        if (n_rcv > 0) {
            next_rcv_valid_index_ += n_rcv;
            logger_.log("%:% %() % read socket:% len:%\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), socket_fd_, next_rcv_valid_index_);
            recv_callback_(this);
        }

        if (next_send_valid_index_ > 0) {
            ssize_t n = ::send(socket_fd_, outbound_data_.data(), next_send_valid_index_, MSG_DONTWAIT | MSG_NOSIGNAL);
            logger_.log("%:% %() % send socket:% len:%\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), socket_fd_, n);
        }
        next_send_valid_index_ = 0;

        return (n_rcv > 0);
    }

    auto MulticastSocket::send(const void *data, size_t len) noexcept -> void {
        memcpy(outbound_data_.data() + next_send_valid_index_, data, len);
        next_send_valid_index_ += len;
        ASSERT(next_send_valid_index_ < MulticastBufferSize, "Mcast socket buffer filled up and sendAndRecv() not called.");
    }
}