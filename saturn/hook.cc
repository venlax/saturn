#include "hook.h"


#include <cerrno>
#include <cstdarg>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <utility>

#include "log.h"
#include "fdmanager.h"
#include "fiber.h"
#include "iomanager.h"
#include "macro.h"
#include "util.h"

namespace saturn {
    Logger::ptr g_logger = LOGGER("system");
    static thread_local bool t_hook_enable = false;

    #define HOOK_FUN(OP) \
        OP(sleep) \
        OP(usleep) \
        OP(nanosleep) \
        OP(socket) \
        OP(connect) \
        OP(accept) \
        OP(read) \
        OP(readv) \
        OP(recv) \
        OP(recvfrom) \
        OP(recvmsg) \
        OP(write) \
        OP(writev) \
        OP(send) \
        OP(sendto) \
        OP(sendmsg) \
        OP(close) \
        OP(fcntl) \
        OP(ioctl) \
        OP(getsockopt) \
        OP(setsockopt)
    void hook_init() {
        static bool is_inited = false;
        if(is_inited) {
            return;
        }
    #define OP(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
        HOOK_FUN(OP);
    #undef OP
    }
    struct HookIniter {
        HookIniter() {
            hook_init();
        }
    };

    static HookIniter hook_initer;

    bool is_hook_enable() {return t_hook_enable;}

    void set_hook_enable(bool flag) {t_hook_enable = flag;}

    struct timer_info {
        int cancelled = 0;
    };

    template<typename Func, typename... Args>
    static ssize_t io(int fd, Func fun, const char* hook_fun_name,
        uint32_t event, int timeout_so, Args&&... args) {
        if (!t_hook_enable) {
            return fun(fd, std::forward<Args>(args)...);
        }
        FdContext::ptr ctx = FdManager::GetInstance()->get(fd); 

        if(!ctx) {
            return fun(fd, std::forward<Args>(args)...);
        }

        if(ctx->isClose()) {
            errno = EBADF;
            return -1;
        }
    
        if(!ctx->isSocket() || ctx->getUserNonblock()) {
            return fun(fd, std::forward<Args>(args)...);
        }
        uint64_t to = ctx->getTimeout(timeout_so);
        std::shared_ptr<timer_info> tinfo(new timer_info);
    retry:
        ssize_t rt;
        while ((rt = fun(fd, std::forward<Args>(args)...)) == -1 && errno == EINTR);
        if (rt == -1 && errno == EAGAIN) {
            IOManager* iom = IOManager::GetThis();
            Timer::ptr timer;
            std::weak_ptr<timer_info> winfo(tinfo);

            if(to != (uint64_t)-1) {
                timer = iom->addConditionTimer(to, [winfo, fd, iom, event]() {
                    auto t = winfo.lock();
                    if(!t || t->cancelled) {
                        return;
                    }
                    t->cancelled = ETIMEDOUT;
                    iom->cancelEvent(fd, (IOManager::Event)(event));
                }, winfo);
            }
            int rt = iom->addEvent(fd, (IOManager::Event)(event));
            if(rt) {
                SATURN_LOG_ERROR(g_logger) << hook_fun_name << " addEvent("
                    << fd << ", " << event << ")";
                if(timer) {
                    timer->cancel();
                }
                return -1;
            } else {
                Fiber::YieldToHold();
                if(timer) {
                    timer->cancel();
                }
                if(tinfo->cancelled) {
                    errno = tinfo->cancelled;
                    return -1;
                }
                goto retry;
            }
        }
        return rt;
    }
}



extern "C" {
    #define OP(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(OP);
    #undef OP

    unsigned int sleep(unsigned int seconds) {
        if (!saturn::t_hook_enable) {
            return sleep_f(seconds);
        }
        saturn::Fiber::ptr fiber = saturn::Fiber::GetThis();
        saturn::IOManager* iom = saturn::IOManager::GetThis();
        iom->addTimer(seconds * 1000, std::bind((void(saturn::Scheduler::*)
                (saturn::Fiber::ptr, int thread))&saturn::IOManager::schedule
                ,iom, fiber, -1));
        saturn::Fiber::YieldToHold();
        return 0;
    }

    int usleep(useconds_t usec) {
        if (!saturn::t_hook_enable) {
            return usleep_f(usec);
        }
        saturn::Fiber::ptr fiber = saturn::Fiber::GetThis();
        saturn::IOManager* iom = saturn::IOManager::GetThis();
        iom->addTimer(usec * 1000, std::bind((void(saturn::Scheduler::*)
                (saturn::Fiber::ptr, int thread))&saturn::IOManager::schedule
                ,iom, fiber, -1));
        saturn::Fiber::YieldToHold();
        return 0;
    }

    int nanosleep(const struct timespec *req, struct timespec *rem) {
        if (!saturn::t_hook_enable) {
            return nanosleep_f(req, rem);
        }
        int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 /1000;
        saturn::Fiber::ptr fiber = saturn::Fiber::GetThis();
        saturn::IOManager* iom = saturn::IOManager::GetThis();
        iom->addTimer(timeout_ms, std::bind((void(saturn::Scheduler::*)
                (saturn::Fiber::ptr, int thread))&saturn::IOManager::schedule
                ,iom, fiber, -1));
        saturn::Fiber::YieldToHold();
        return 0;
    }

    int socket(int domain, int type, int protocol) {
        if(!saturn::t_hook_enable) {
            return socket_f(domain, type, protocol);
        }
        int fd = socket_f(domain, type, protocol);
        if(fd == -1) {
            return fd;
        }
        saturn::FdManager::GetInstance()->get(fd, true);
        return fd;
    }

    int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms) {
        if(!saturn::t_hook_enable) {
            return connect_f(fd, addr, addrlen);
        }
        saturn::FdContext::ptr ctx = saturn::FdManager::GetInstance()->get(fd);
        if(!ctx || ctx->isClose()) {
            errno = EBADF;
            return -1;
        }
    
        if(!ctx->isSocket()) {
            return connect_f(fd, addr, addrlen);
        }
    
        if(ctx->getUserNonblock()) {
            return connect_f(fd, addr, addrlen);
        }
    
        int n = connect_f(fd, addr, addrlen);
        if(n == 0) {
            return 0;
        } else if(n != -1 || errno != EINPROGRESS) {
            return n;
        }
    
        saturn::IOManager* iom = saturn::IOManager::GetThis();
        saturn::Timer::ptr timer;
        std::shared_ptr<saturn::timer_info> tinfo(new saturn::timer_info);
        std::weak_ptr<saturn::timer_info> winfo(tinfo);
    
        if(timeout_ms != (uint64_t)-1) {
            timer = iom->addConditionTimer(timeout_ms, [winfo, fd, iom]() {
                    auto t = winfo.lock();
                    if(!t || t->cancelled) {
                        return;
                    }
                    t->cancelled = ETIMEDOUT;
                    iom->cancelEvent(fd, saturn::IOManager::WRITE);
            }, winfo);
        }
    
        int rt = iom->addEvent(fd, saturn::IOManager::WRITE);
        if(rt == 0) {
            saturn::Fiber::YieldToHold();
            if(timer) {
                timer->cancel();
            }
            if(tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }
        } else {
            if(timer) {
                timer->cancel();
            }
            SATURN_LOG_ERROR(saturn::g_logger) << "connect addEvent(" << fd << ", WRITE) error";
        }
    
        int error = 0;
        socklen_t len = sizeof(int);
        if(-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) {
            return -1;
        }
        if(!error) {
            return 0;
        } else {
            errno = error;
            return -1;
        }
    }
    

    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
        return connect_with_timeout(sockfd, addr, addrlen, 5000);
    }
    
    int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
        int fd = io(s, accept_f, "accept", saturn::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
        if(fd >= 0) {
            saturn::FdManager::GetInstance()->get(fd, true);
        }
        return fd;
    }

    ssize_t read(int fd, void *buf, size_t count) {
        return io(fd, read_f, "read", saturn::IOManager::READ, SO_RCVTIMEO, buf, count);
    }

    ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
        return io(fd, readv_f, "readv", saturn::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);    
    }

    ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
        return io(sockfd, recv_f, "recv", saturn::IOManager::READ, SO_RCVTIMEO, buf, len, flags);    
    }
     
    ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
        return io(sockfd, recvfrom_f, "recvfrom", saturn::IOManager::READ,SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
    }

    ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
        return io(sockfd, recvmsg_f, "recvmsg", saturn::IOManager::READ, SO_RCVTIMEO, msg, flags);
    }
    
    ssize_t write(int fd, const void *buf, size_t count) {
        return io(fd, write_f, "write", saturn::IOManager::WRITE, SO_SNDTIMEO, buf, count);
    }
    
    ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
        return io(fd, writev_f, "writev", saturn::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
    }

    ssize_t send(int s, const void *msg, size_t len, int flags) {
        return io(s, send_f, "send", saturn::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
    }
 
    ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
        return io(s, sendto_f, "sendto", saturn::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to , tolen);
    }
 
    ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
        return io(s, sendmsg_f, "sendmsg", saturn::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
    }

    int close(int fd) {
        if (!saturn::t_hook_enable) {
            return close_f(fd);
        }
        saturn::FdContext::ptr ptr = saturn::FdManager::GetInstance()->get(fd);
        if (ptr) {
            auto iom = saturn::IOManager::GetThis();
            if(iom) {
                iom->cancelAll(fd);
            }
            saturn::FdManager::GetInstance()->del(fd);
        }
        return close_f(fd);
    }

    // ctl
    int fcntl(int fd, int cmd, ... /* arg */ ) {
        va_list va;
        va_start(va, cmd);
        switch (cmd) {
            case F_SETFL: {
                int arg = va_arg(va, int);
                va_end(va);
                saturn::FdContext::ptr ctx = saturn::FdManager::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return fcntl_f(fd, cmd, arg);
                }
                ctx->setUserNonblock(arg & O_NONBLOCK);
                if(ctx->getSysNonblock()) {
                    arg |= O_NONBLOCK;
                } else {
                    arg &= ~O_NONBLOCK;
                }
                return fcntl_f(fd, cmd, arg);
            }
            break;
            case F_GETFL:
            {
                va_end(va);
                int arg = fcntl_f(fd, cmd);
                saturn::FdContext::ptr ctx = saturn::FdManager::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return arg;
                }
                if(ctx->getUserNonblock()) {
                    return arg | O_NONBLOCK;
                } else {
                    return arg & ~O_NONBLOCK;
                }
            }
            break;
            case F_DUPFD:
            case F_DUPFD_CLOEXEC:
            case F_SETFD:
            case F_SETOWN:
            case F_SETSIG:
            case F_SETLEASE:
            case F_NOTIFY:
    #ifdef F_SETPIPE_SZ
            case F_SETPIPE_SZ:
    #endif
                {
                    int arg = va_arg(va, int);
                    va_end(va);
                    return fcntl_f(fd, cmd, arg); 
                }
                break;
            case F_GETFD:
            case F_GETOWN:
            case F_GETSIG:
            case F_GETLEASE:
    #ifdef F_GETPIPE_SZ
            case F_GETPIPE_SZ:
    #endif
                {
                    va_end(va);
                    return fcntl_f(fd, cmd);
                }
                break;
            case F_SETLK:
            case F_SETLKW:
            case F_GETLK:
                {
                    struct flock* arg = va_arg(va, struct flock*);
                    va_end(va);
                    return fcntl_f(fd, cmd, arg);
                }
                break;
            case F_GETOWN_EX:
            case F_SETOWN_EX:
                {
                    struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                    va_end(va);
                    return fcntl_f(fd, cmd, arg);
                }
                break;
            default:
                va_end(va);
                return fcntl_f(fd, cmd);
        }
        return -1;
    }


    int ioctl(int d, unsigned long int request, ...) {
        va_list va;
        va_start(va, request);
        void* arg = va_arg(va, void*);
        va_end(va);
    
        if(FIONBIO == request) {
            bool user_nonblock = !!*(int*)arg;
            saturn::FdContext::ptr ctx = saturn::FdManager::GetInstance()->get(d);
            if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                return ioctl_f(d, request, arg);
            }
            ctx->setUserNonblock(user_nonblock);
        }
        return ioctl_f(d, request, arg);
    }

    int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
        return getsockopt_f(sockfd, level, optname, optval, optlen);
    }

    int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
        if (!saturn::t_hook_enable) {
            return setsockopt_f(sockfd, level, optname, optval, optlen);
        }
        if(level == SOL_SOCKET) {
            if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
                saturn::FdContext::ptr ctx = saturn::FdManager::GetInstance()->get(sockfd);
                if(ctx) {
                    const timeval* v = (const timeval*)optval;
                    ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
                }
            }
        }
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
}