#include "logging.h"
#include "tcp_socket.h"

namespace Common {
    auto TCPSocket::destory() noexcept -> void {
        close(fd_);
        fd_ = -1;
    };

    auto TCPSocket::connect(const std::string &ip, const std::string &iface, int port, bool is_listening) -> int {
        destory();
        fd_ = createSocket(logger_, ip, iface, port, false, false, is_listening, 0, true);
        
        // Set local info in TCPSocket obj
        // Bind to all local addresses
        inInAddr.sin_addr.s_addr = INADDR_ANY;
        // set port
        inInAddr.sin_port = htons(port);
        //AF_INET = IPv4
        inInAddr.sin_family = AF_INET;

        return fd_;
    };

    auto TCPSocket::send(const void *data, size_t len) noexcept -> void {
        if(len > 0){
            memcpy(send_buffer_ + next_send_valid_index_, data, len);
            next_send_valid_index_ += len;
        }
    }

    auto TCPSocket::sendAndRecv() noexcept -> bool { 

        // Buffor for ancillary data - 
        char ctrl[CMSG_SPACE(sizeof(struct timeval))];
        struct cmsghdr *cmsg = (struct cmsghdr *) &ctrl;

        // I/O vector. Struct that contains pointer to start and length
        // we use it as a buffor for data to send
        struct iovec iov;
        iov.iov_base = rcv_buffer_ + next_rcv_valid_index_;
        iov.iov_len = TCPBufferSize - next_rcv_valid_index_;

        msghdr msg;
        msg.msg_control = ctrl;
        msg.msg_controllen = sizeof(ctrl);
        msg.msg_name = &inInAddr;
        msg.msg_namelen = sizeof(inInAddr);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        // recvmsg - writes to many places at once
        // msg.msg_iov -> buffor for incoming message
        // msg.msg_name -> place for sender info
        // msg.msg_control -> timestamp added by kernel
        const auto n_rcv = recvmsg(fd_, &msg, MSG_DONTWAIT);
        if(n_rcv > 0) {
            next_rcv_valid_index_ += n_rcv;

            Nanos kernel_time = 0;
            struct timeval time_kernel;

            // Checking if this control message (cmsg) from recvmsg is timestamp from kernel for socket
            if(cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_TIMESTAMP && cmsg->cmsg_len == CMSG_LEN(sizeof(time_kernel))) {
                memcpy(&time_kernel, CMSG_DATA(cmsg), sizeof(time_kernel));

                kernel_time = time_kernel.tv_sec * NANOS_TO_SECS + time_kernel.tv_usec * NANOS_TO_MICROS;
            }

            const auto user_time = getCurrentNanos();
            logger_.log("%:% %() % read socket:% len:% usrTime:% kernelTime:% diffTime: %\n",
                __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), fd_, next_rcv_valid_index_, user_time, kernel_time, (user_time - kernel_time));
            
            recv_callback_(this, kernel_time);
        }

        //ssize_t - singed type of size_t, also stores any size of eny obj
        // We don't send more than buffer limit
        ssize_t n_send = std::min(TCPBufferSize, next_send_valid_index_);
        while(n_send > 0) {
            auto n_send_this_msg = std::min(static_cast<ssize_t>(next_send_valid_index_), n_send);

            // MSG_DONTWAIT - flag for non blocking
            // MSG_NOSIGNAL - when pper closed, normal error
            // MSG_MORE - helps kernel with packages
            const int flags = MSG_DONTWAIT | MSG_NOSIGNAL | (n_send_this_msg < n_send ? MSG_MORE : 0);
            auto n = ::send(fd_, send_buffer_, n_send_this_msg, flags);

            // if ::send returned -1 then we need to stop
            if(n < 0){
                // When nonblocking, we got errors, if different than EAGAIN / EWOULDBLOCK we close
                if(!wouldBlock()){
                    send_disconnected_ = true;
                }
            }

            logger_.log("%:% %() % read socket:% len:% \n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), fd_, next_rcv_valid_index_);

            n_send -= n;
        }

        next_send_valid_index_ = 0;
        return (n_rcv > 0);                
    }
}
