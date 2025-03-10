#ifndef __SATURN_HOOK_H__
#define __SATURN_HOOK_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>


namespace saturn {

    bool is_hook_enable();

    void set_hook_enable(bool flag);
}
extern "C" {
    // sleep
    using sleep_fun = unsigned int (*) (unsigned int seconds);
    extern sleep_fun sleep_f;
    
    using usleep_fun = int (*)(useconds_t usec);
    extern usleep_fun usleep_f;

    using nanosleep_fun = int (*)(const struct timespec *req, struct timespec *rem);
    extern nanosleep_fun nanosleep_f;
    // socket
    using socket_fun = int (*)(int domain, int type, int protocol);
    extern socket_fun socket_f;

    using connect_fun = int (*)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    extern connect_fun connect_f;

    using accept_fun = int (*)(int s, struct sockaddr *addr, socklen_t *addrlen);
    extern accept_fun accept_f;

    //read
    using read_fun = ssize_t (*)(int fd, void *buf, size_t count);
    extern read_fun read_f;

    using readv_fun = ssize_t (*)(int fd, const struct iovec *iov, int iovcnt);
    extern readv_fun readv_f;

    using recv_fun = ssize_t (*)(int sockfd, void *buf, size_t len, int flags);
    extern recv_fun recv_f;

    using recvfrom_fun =  ssize_t (*)(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
    extern recvfrom_fun recvfrom_f;

    using recvmsg_fun =  ssize_t (*)(int sockfd, struct msghdr *msg, int flags);
    extern recvmsg_fun recvmsg_f;

    //write
    using write_fun = ssize_t (*)(int fd, const void *buf, size_t count);
    extern write_fun write_f;

    using writev_fun = ssize_t (*)(int fd, const struct iovec *iov, int iovcnt);
    extern writev_fun writev_f;

    using send_fun = ssize_t (*)(int s, const void *msg, size_t len, int flags);
    extern send_fun send_f;

    using sendto_fun = ssize_t (*)(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);
    extern sendto_fun sendto_f;

    using sendmsg_fun =  ssize_t (*)(int s, const struct msghdr *msg, int flags);
    extern sendmsg_fun sendmsg_f;

    using close_fun  = int (*)(int fd);
    extern close_fun close_f;

    // ctl
    using fcntl_fun = int (*)(int fd, int cmd, ... /* arg */ );
    extern fcntl_fun fcntl_f;

    using ioctl_fun =  int (*)(int d, unsigned long int request, ...);
    extern ioctl_fun ioctl_f;

    using getsockopt_fun = int (*)(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
    extern getsockopt_fun getsockopt_f;

    using setsockopt_fun =  int (*)(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
    extern setsockopt_fun setsockopt_f;

    int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms);
}

#endif // !__SATURN_HOOK_H__