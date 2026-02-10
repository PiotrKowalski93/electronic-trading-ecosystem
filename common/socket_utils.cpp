#include "socket_utils.h"

// A network interface is the boundary between the kernel’s networking stack and the physical hardware that transmits data over the network.
// A socket is a software abstraction for communication, while a network interface is the boundary where the kernel sends data to the physical network hardware.

namespace Common{
    // Maps network interface to IPv4 without DNS, so network is not used.
    // It works locally
    auto getIfaceIP(const std::string &iface) -> std::string{
        char buf[NI_MAXHOST] = {'\0'};
        ifaddrs *ifaddr = nullptr;

        // Iterate throught network interfaces and finds 'iface' ex. eth0
        if (getifaddrs(&ifaddr) != -1) 
        {
            for (ifaddrs *ifa = ifaddr; ifa; ifa = ifa->ifa_next) 
            {
                if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET && iface == ifa->ifa_name) 
                {
                    // translates to ip 123.123.123.123
                    getnameinfo(ifa->ifa_addr, sizeof(sockaddr_in), buf, sizeof(buf), NULL, 0, NI_NUMERICHOST);
                    break;
                }
            }
            freeifaddrs(ifaddr);
        }

        return buf;
    }

    // By default kernel sets to blocking
    auto setNonBlocking(int fd) -> bool{
        const auto flags = fcntl(fd, F_GETFL, 0);

        if (flags & O_NONBLOCK) return true;

        // fcntl sets socket to non-blocking
        return (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1);
    }

    // Disabling Nagle's algorithm for TCP
    auto setNoDelay(int fd) -> bool {
        int one = 1;
        // We do reinterpret_cast because C api does not know C++ types, kernel operates on bytes, void* is pointer to "some" memory
        return (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<void *>(&one), sizeof(one)) != -1);
    }
    
    // Attach timestamp to every message, timestamp when packet got to kernel
    // timestamp will be visible in recvmsg()
    // Microsecond precision
    auto setSOTimestamp(int fd) -> bool {
        int one = 1; // 0 = off
        return(setsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, reinterpret_cast<void*>(&one), sizeof(one)) != -1);
    }

    // Nanosecond precision
    auto setSOTimestampNs(int fd) -> bool {
        int one = 1;
        return(setsockopt(fd, SOL_SOCKET, SO_TIMESTAMPNS, reinterpret_cast<void*>(&one), sizeof(one)) != -1);
    }

    // Check if socket blocks, we need to check global variable
    // errno
    auto wouldBlock() -> bool{
        return (errno == EWOULDBLOCK || errno == EINPROGRESS);
    }

    // TTL - Time to Live: network level setting, controls max number of hops between sender and reciever
    auto setTTL(int fd, int ttl) -> bool {
        return(setsockopt(fd, IPPROTO_TCP, IP_TTL, reinterpret_cast<void*>(&ttl), sizeof(ttl)) != -1);
    }

    auto setMcastTTL(int fd, int mcast_ttl) noexcept -> bool {
        return(setsockopt(fd, IPPROTO_TCP, IP_MULTICAST_TTL, reinterpret_cast<void*>(&mcast_ttl), sizeof(mcast_ttl)) != -1);
    }
    
    auto join(int fd, const std::string &ip, const std::string &iface, int port) -> bool;

    //TODO: Refactor method. Break it into utp, tcp, listening, sending. SRP bro
    auto createSocket(Logger &logger, const std::string &t_ip, const std::string &iface, int port, bool is_udp, bool is_blocking, bool is_listening, int ttl, bool needs_so_timestamp) -> int {
        std::string time_str;
        
        const auto ip = t_ip.empty() ? getIfaceIP(iface) : t_ip;

        logger.log("%:% %() % ip:% iface:% poty:% isUdp:% isBlocking:% isListening:% ttl:% SO_time:%",
                __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str), ip, iface, port, is_udp, is_blocking, is_listening, ttl, needs_so_timestamp);

        addrinfo hints {};
        hints.ai_family = AF_INET;
        hints.ai_socktype = is_udp ? SOCK_DGRAM : SOCK_STREAM;
        hints.ai_protocol = is_udp ? IPPROTO_UDP : IPPROTO_TCP;
        hints.ai_flags = is_listening ? AI_PASSIVE : 0;
        if(std::isdigit(ip.c_str()[0])) hints.ai_flags |= AI_NUMERICHOST;
        hints.ai_flags |= AI_NUMERICSERV;

        //getaddrinfo() resolves a logical endpoint (IP/port/protocol intent)
        //into one or more concrete socket addresses usable by the OS.
        addrinfo *result = nullptr;
        const auto rc = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &result);

        if(rc){
            // TODO: Should i use strerror_s? if yes, how
            logger.log("Get getaddrinfo failed. error:% \n", strerror(errno));
            return -1;
        }

        int fd = -1;
        int one = 1;

        // rp - result pointer
        for(addrinfo *rp = result; rp; rp = rp->ai_next) {
            fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

            if(fd == -1) {
                logger.log("socket() failed. errno:%", strerror(errno));
                return -1;
            } 

            // Try setting no blocking and no delay
            // We have to set non-blocking and delay before connect() !
            if(!is_blocking){
                if(!setNonBlocking(fd)){
                    logger.log("setNonBlocking() failed. errno:%", strerror(errno));
                    return -1;
                }
                // Not for UDP because it does not have Nagle;s Algo
                if(!is_udp && !setNoDelay(fd)){
                    logger.log("setNoDelay() failed. errno:%", strerror(errno));
                    return -1;
                }
            }

            // Is not server and was able to connect = ok
            if(!is_listening && connect(fd, rp->ai_addr, rp->ai_addrlen) == 1 && !wouldBlock()) {
                logger.log("connect() failed. errno:%", strerror(errno));
                return -1;
            }

            // SO_REUSEADDR only for server. We need to set it before bind()
            if(is_listening && setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<void*>(&one), sizeof(one)) == -1){
                logger.log("setsockopt() failed. errno:%", strerror(errno));
                return -1;
            }

            //Is server, was able to bind = ok
            if(is_listening && bind(fd, rp->ai_addr, rp->ai_addrlen) == -1) {
                logger.log("bind() failed. errno:%", strerror(errno));
                return -1;
            }

            // If not udp and is server, then listen
            if(!is_udp && is_listening && listen(fd, MaxTCPServerBacklog) == -1){
                logger.log("listen() failed. errno:%", strerror(errno));
                return -1;
            }

            if(is_udp && ttl){
                const bool is_multicast = atoi(ip.c_str()) & 0xe0;
                if(is_multicast && !setMcastTTL(fd, ttl)){
                    logger.log("setMcastTTL() failed. errno:%", strerror(errno));
                    return -1;
                }
                if(!is_multicast && !setTTL(fd, ttl)){
                    logger.log("setMcastTTL() failed. errno:%", strerror(errno));
                    return -1;
                }
            }

            if(needs_so_timestamp && !setSOTimestamp(fd)){
                logger.log("setSOTimestamp() failed. errno:%", strerror(errno));
                return -1;
            }
        }

        if(result) {
            freeaddrinfo(result);
        }

        return fd;
    }  
}