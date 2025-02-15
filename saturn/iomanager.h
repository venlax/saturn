#ifndef __SATURN_IOMANAGER_H__
#define __SATURN_IOMANAGER_H__


#include "scheduler.h"


#include <atomic>
#include <functional>
#include <shared_mutex>
#include <vector>

namespace saturn{
    class IOManager : public Scheduler {
    public:
        using ptr = std::shared_ptr<IOManager>;
        enum Event {
            NONE    = 0x0,
            READ    = 0x1,
            WRITE   = 0x4
        }; // not use enum class to support bool operand
    private:
        struct FdContext {
            struct EventContext {
                Scheduler* scheduler = nullptr;
                Fiber::ptr fiber;
                std::function<void()> cb;
            };

            EventContext& getContext(Event event);

            void resetContext(EventContext& ctx);

            void triggerEvent(Event event);

            EventContext read;
            EventContext write;

            int fd = 0;
            
            Event events = Event::NONE;

            std::mutex m_mutex;
        };
    public:
        IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "");
        ~IOManager();
        int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
        bool delEvent(int fd, Event event);
        bool cancelEvent(int fd, Event event);
        bool cancelAll(int fd);
        
        static IOManager* GetThis();
    protected:
        void tickle() override;
        bool stopping() override;
        void idle() override;

        void contextResize(size_t size);
        bool stopping(uint64_t& time_out);
    private:
        int m_epfd = 0;
        int m_tickleFds[2];
        std::atomic<size_t> m_pendingEventCount = {0};
        std::shared_mutex m_mutex;
        std::vector<FdContext*> m_fdContexts;
    };
}


#endif // !__SATURN_IOMANAGER_H__