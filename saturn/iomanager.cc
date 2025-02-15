#include "iomanager.h"
#include "fiber.h"
#include "log.h"
#include "macro.h"
#include "util.h"

#include <fcntl.h>
#include <mutex>
#include <shared_mutex>
#include <sys/epoll.h>
#include <unistd.h>

namespace saturn {
    static Logger::ptr g_logger = LOGGER("system");

    IOManager::IOManager(size_t threads, bool use_caller, const std::string& name) 
    : Scheduler(threads, use_caller, name) {
        m_epfd = epoll_create(1000);
        SATURN_ASSERT(m_epfd >= 0);
        int rt = pipe(m_tickleFds);
        SATURN_ASSERT(!rt);

        epoll_event event;
        memset(&event, 0, sizeof(epoll_event));
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = m_tickleFds[0];

        rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
        SATURN_ASSERT(!rt);
    
        rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
        SATURN_ASSERT(!rt);
    
        contextResize(32);
    
        start();

    }
    IOManager::~IOManager() {
        stop();
        close(m_epfd);
        close(m_tickleFds[0]);
        close(m_tickleFds[1]);
    
        for(size_t i = 0; i < m_fdContexts.size(); ++i) {
            if(m_fdContexts[i]) {
                delete m_fdContexts[i];
            }
        }
    }


    IOManager::FdContext::EventContext& IOManager::FdContext::getContext(Event event) {
        if (event == READ) {
            return read;
        } else if (event == WRITE) {
            return write;
        }
        SATURN_ASSERT(false);
    }

    void IOManager::FdContext::resetContext(EventContext& ctx) {
        ctx.cb = nullptr;
        ctx.fiber.reset();
        ctx.scheduler = nullptr;
    }

    void IOManager::FdContext::triggerEvent(Event event) {
        SATURN_ASSERT(events & event);
        events = (Event)(events & ~event);
        EventContext& ctx = getContext(event);
        if(ctx.cb) {
            ctx.scheduler->schedule(&ctx.cb);
        } else {
            ctx.scheduler->schedule(&ctx.fiber);
        }
        ctx.scheduler = nullptr;
        return;
    }


    int IOManager::addEvent(int fd, Event event, std::function<void()> cb) {
        FdContext* fc = nullptr;
        {
            LOCK(shared_lock, m_mutex);
            if (m_fdContexts.size() > fd) {
                fc = m_fdContexts[fd];
                UNLOCK();
            } else {    
                UNLOCK();
                LOCK(unique_lock, m_mutex);
                contextResize(fd * 2); // same as libstdc++ vector resize strategy
                fc = m_fdContexts[fd];
            }
        }
        LOCK(lock_guard, fc->m_mutex);
        if (fc->events & event) {
            SATURN_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd
            << " event=" << (EPOLL_EVENTS) event
            << " fc.event=" << (EPOLL_EVENTS) fc->events;
            SATURN_ASSERT(false);
        }

        int op = fc->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
        epoll_event epevent;
        epevent.events = EPOLLET | fc->events | event;
        epevent.data.ptr = fc;
        int rt = epoll_ctl(m_epfd, op, fd, &epevent);

        if(rt) {
            SATURN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                << op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
                << rt << " (" << errno << ") (" << strerror(errno) << ") fc->events="
                << (EPOLL_EVENTS)fc->events;
            return -1;
        }

        ++m_pendingEventCount;
        fc->events = (Event)(fc->events | event);
        FdContext::EventContext& event_ctx = fc->getContext(event);
        SATURN_ASSERT(!event_ctx.scheduler
                    && !event_ctx.fiber
                    && !event_ctx.cb);
    
        event_ctx.scheduler = Scheduler::GetThis();
        if(cb) {
            event_ctx.cb.swap(cb);
        } else {
            event_ctx.fiber = Fiber::GetThis();
            SATURN_ASSERT(event_ctx.fiber->getState() == Fiber::State::EXEC);
        }
        return 0;
    }

    bool IOManager::delEvent(int fd, Event event) {
        FdContext* fc = nullptr;
        {   
            LOCK(shared_lock, m_mutex);
            if (m_fdContexts.size() > fd) {
                fc = m_fdContexts[fd];
            } else {
                return false;
            }
        
        }
        LOCK(lock_guard, fc->m_mutex);

        if (!(fc->events & event)) 
            return false;
        
        Event new_event = static_cast<Event>(fc->events & ~event);
        int op = new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = EPOLLIN | new_event;
        epevent.data.ptr = fc;
        int rt = epoll_ctl(m_epfd, op, fd, &epevent);
        if(rt) {
            SATURN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                << op << ", " << fd << ", " << (EPOLL_EVENTS) epevent.events << "):"
                << rt << " (" << errno << ") (" << strerror(errno) << ") fc->events="
                << (EPOLL_EVENTS)fc->events;
            return false;
        }
        --m_pendingEventCount;
        fc->events = new_event;
        FdContext::EventContext& event_ctx = fc->getContext(event);
        fc->resetContext(event_ctx);
        return true;
    }
    bool IOManager::cancelEvent(int fd, Event event) {
        FdContext* fc = nullptr;
        {   
            LOCK(shared_lock, m_mutex);
            if (m_fdContexts.size() > fd) {
                fc = m_fdContexts[fd];
            } else {
                return false;
            }
        
        }
        LOCK(lock_guard, fc->m_mutex);

        if (!(fc->events & event)) 
            return false;
        
        Event new_event = static_cast<Event>(fc->events & ~event);
        int op = new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = EPOLLIN | new_event;
        epevent.data.ptr = fc;
        int rt = epoll_ctl(m_epfd, op, fd, &epevent);
        if(rt) {
            SATURN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                << op << ", " << fd << ", " << (EPOLL_EVENTS) epevent.events << "):"
                << rt << " (" << errno << ") (" << strerror(errno) << ") fc->events="
                << (EPOLL_EVENTS)fc->events;
            return false;
        }
        --m_pendingEventCount;
        fc->triggerEvent(event);
        return true;
    }
    bool IOManager::cancelAll(int fd) {
        FdContext* fc = nullptr;
        {   
            LOCK(shared_lock, m_mutex);
            if (m_fdContexts.size() > fd) {
                fc = m_fdContexts[fd];
            } else {
                return false;
            }
        
        }
        LOCK(lock_guard, fc->m_mutex);

        if (!fc->events) 
            return false;
        int op =  EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = 0;
        epevent.data.ptr = fc;
        int rt = epoll_ctl(m_epfd, op, fd, &epevent);
        if(rt) {
            SATURN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                << op << ", " << fd << ", " << (EPOLL_EVENTS) epevent.events << "):"
                << rt << " (" << errno << ") (" << strerror(errno) << ") fc->events="
                << (EPOLL_EVENTS)fc->events;
            return false;
        }

        if(fc->events & READ) {
            fc->triggerEvent(READ);
            --m_pendingEventCount;
        }
        if(fc->events & WRITE) {
            fc->triggerEvent(WRITE);
            --m_pendingEventCount;
        }
    
        SATURN_ASSERT(fc->events == 0);
        return true;
        
    }
    IOManager* IOManager::GetThis() {
        return dynamic_cast<IOManager*>(Scheduler::GetThis());
    }

    void IOManager::tickle() {
        if(!hasIdleThreads()) {
            return;
        }
        int rt = write(m_tickleFds[1], "T", 1);
        SATURN_ASSERT(rt == 1);
    }
    bool IOManager::stopping() {
        uint64_t time_out = 0;
        return stopping(time_out);
    }
    void IOManager::idle() {
        SATURN_LOG_INFO(g_logger) << "idle";
        const uint64_t MAX_EVENTS = 256;
        epoll_event* events = new epoll_event[MAX_EVENTS]();
        std::shared_ptr<epoll_event> shared_events(events, [](epoll_event* ptr){
            delete[] ptr;
        });
        while(true) {
            uint64_t next_timeout = 0;
            if(stopping(next_timeout)) {
                SATURN_LOG_INFO(g_logger) << "name=" << getName()
                                         << " idle stopping exit";
                break;
            }
    
            int rt = 0;
            do {
                static const int MAX_TIMEOUT = 3000;
                if(next_timeout != ~0ull) {
                    next_timeout = (int)next_timeout > MAX_TIMEOUT
                                    ? MAX_TIMEOUT : next_timeout;
                } else {
                    next_timeout = MAX_TIMEOUT;
                }
                rt = epoll_wait(m_epfd, events, MAX_EVENTS, next_timeout);
                if(rt < 0 && errno == EINTR) {
                } else {
                    break;
                }
            } while(true);
    
            std::vector<std::function<void()> > cbs;
            listExpiredCb(cbs);
            if(!cbs.empty()) {
                schedule(cbs.begin(), cbs.end());
                cbs.clear();
            }
    
            for(int i = 0; i < rt; ++i) {
                epoll_event& event = events[i];
                if(event.data.fd == m_tickleFds[0]) {
                    uint8_t dummy[256];
                    while(read(m_tickleFds[0], dummy, sizeof(dummy)) > 0);
                    continue;
                }
    
                FdContext* fc = (FdContext*)event.data.ptr;
                LOCK(lock_guard, fc->m_mutex);
                if(event.events & (EPOLLERR | EPOLLHUP)) {
                    event.events |= (EPOLLIN | EPOLLOUT) & fc->events;
                }
                int real_events = NONE;
                if(event.events & EPOLLIN) {
                    real_events |= READ;
                }
                if(event.events & EPOLLOUT) {
                    real_events |= WRITE;
                }
    
                if((fc->events & real_events) == NONE) {
                    continue;
                }
    
                int left_events = (fc->events & ~real_events);
                int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
                event.events = EPOLLET | left_events;
    
                int rt2 = epoll_ctl(m_epfd, op, fc->fd, &event);
                if(rt2) {
                    SATURN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                        << op << ", " << fc->fd << ", " << (EPOLL_EVENTS)event.events << "):"
                        << rt2 << " (" << errno << ") (" << strerror(errno) << ")";
                    continue;
                }
                if(real_events & READ) {
                    fc->triggerEvent(READ);
                    --m_pendingEventCount;
                }
                if(real_events & WRITE) {
                    fc->triggerEvent(WRITE);
                    --m_pendingEventCount;
                }
            }
    
            Fiber::ptr cur = Fiber::GetThis();
            auto raw_ptr = cur.get();
            cur.reset();
    
            raw_ptr->swapOut();
        }
    }

    void IOManager::contextResize(size_t size) {
        m_fdContexts.resize(size);
    
        for(size_t i = 0; i < m_fdContexts.size(); ++i) {
            if(!m_fdContexts[i]) {
                m_fdContexts[i] = new FdContext;
                m_fdContexts[i]->fd = i;
            }
        }
    }

    bool IOManager::stopping(uint64_t& time_out) {
        return time_out == ~0ull
            && m_pendingEventCount == 0
            && Scheduler::stopping();
    }

    void IOManager::onTimerInsertedAtFront() {
        tickle();
    }
}