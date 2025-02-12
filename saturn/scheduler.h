#ifndef __SATURN_SCHEDULER_H__
#define __SATURN_SCHEDULER_H__

#include <atomic>
#include <list>
#include <mutex>
#include <sched.h>
#include <vector>

#include "macro.h"
#include "thread.h"
#include "fiber.h"


namespace saturn {
    class Scheduler {
    public:
        Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "");


        virtual ~Scheduler();


        const std::string& getName() const { return m_name;}


        static Scheduler* GetThis();

        static Fiber* GetMainFiber();


        void start();

        void stop();

        template<class FiberOrCb>
        void schedule(FiberOrCb fc, pid_t thread = -1) {
            bool need_tickle = false;
            {
                LOCK(lock_guard, m_mutex);
                need_tickle = scheduleNoLock(fc, thread);
            }

            if(need_tickle) {
                tickle();
            }
        }


        template<class InputIterator>
        void schedule(InputIterator begin, InputIterator end) {
            bool need_tickle = false;
            {
                LOCK(lock_guard, m_mutex);
                while(begin != end) {
                    need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;
                    ++begin;
                }
            }
            if(need_tickle) {
                tickle();
            }
        }

        void switchTo(int thread = -1);
        std::ostream& dump(std::ostream& os);
    protected:

        virtual void tickle();

        void run();


        virtual bool stopping();


        virtual void idle();


        void setThis();


        bool hasIdleThreads() { return m_idleThreadCount > 0;}
    private:

        template<class FiberOrCb>
        bool scheduleNoLock(FiberOrCb fc, pid_t thread) {
            bool need_tickle = m_fibers.empty();
            FiberAndThread ft(fc, thread);
            if(ft.fiber || ft.cb) {
                m_fibers.push_back(ft);
            }
            return need_tickle;
        }
    

    private:
        class FiberAndThread {

        public:
            Fiber::ptr fiber;
            std::function<void()> cb;
            pid_t thread;

            FiberAndThread(Fiber::ptr f, pid_t thr)
                :fiber(f), thread(thr) {
            }
            FiberAndThread(Fiber::ptr* f, pid_t thr)
                :thread(thr) {
                fiber.swap(*f);
            }

            FiberAndThread(std::function<void()> f, pid_t thr)
                :cb(f), thread(thr) {
            }

            FiberAndThread(std::function<void()>* f, pid_t thr)
                :thread(thr) {
                cb.swap(*f);
            }

            FiberAndThread()
                :thread(-1) {
            }

            void reset() {
                fiber = nullptr;
                cb = nullptr;
                thread = -1;
            }
        };
        std::mutex m_mutex;
        std::vector<Thread::ptr> m_threads;
        std::list<FiberAndThread> m_fibers;
        Fiber::ptr m_rootFiber;
        std::string m_name;
    protected:
        std::vector<pid_t> m_threadIds;
        size_t m_threadCount = 0;
        std::atomic<size_t> m_activeThreadCount = {0};
        std::atomic<size_t> m_idleThreadCount = {0};
        bool m_stopping = true;
        bool m_autoStop = false;
        pid_t m_rootThread = 0;
    };
}

#endif // !__SATURN_SCHEDULER_H__
