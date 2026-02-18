#include <iostream>
#include <string.h>
#include <unordered_set>
#include <sstream>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#include "logging.h"
#include "macros.h"

namespace Common{
    constexpr int MaxTCPServerBacklog = 1024;

    auto getIfaceIP(const std::string &iface) -> std::string;
    auto setNonBlocking(int fd) -> bool;
    auto setNoDelay(int fd) -> bool;
    auto setSOTimestamp(int fd) -> bool;
    auto setSOTimestampNs(int fd) -> bool;
    auto wouldBlock() -> bool;
    auto setTTL(int fd, int ttl) -> bool;
    auto setMcastTTL(int fd, int mcast_ttl) noexcept -> bool;
    auto join(int fd, const std::string &ip) -> bool;
    auto createSocket(Logger &logger, const std::string &t_ip, const std::string &iface, int port, bool is_udp, bool is_blocking, bool is_listening, int ttl, bool needs_so_timestamp) -> int; 

    struct SocketCfg {
        std::string ip_;
        std::string iface_;
        int port_ = -1;
        bool is_udp_ = false;
        bool is_listening_ = false;
        bool needs_so_timestamp_ =  false;

        auto toString() const {
        std::stringstream ss;
        ss << "SocketCfg[ip:" << ip_
        << " iface:" << iface_
        << " port:" << port_
        << " is_udp:" << is_udp_
        << " is_listening:" << is_listening_
        << " needs_SO_timestamp:" << needs_so_timestamp_
        << "]";

        return ss.str();
        }
    };
}