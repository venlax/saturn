#include "scheduler.h"

#include "fiber.h"
#include "log.h"
#include "macro.h"
#include "util.h"
#include <mutex>

namespace saturn {

    static Logger::ptr g_logger = LOGGER("system");

    static thread_local Scheduler* t_scheduler = nullptr;
    static thread_local Fiber* t_scheduler_fiber = nullptr;

    Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name):
        m_name(name) {
        if (use_caller) {
            Fiber::GetThis();
            threads--;

            SATURN_ASSERT(GetThis() == nullptr);
            t_scheduler = this;

            m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
            Thread::SetName(m_name);
    
            t_scheduler_fiber = m_rootFiber.get();
            m_rootThread = getThreadId();
            m_threadIds.push_back(m_rootThread);
        } else {
            m_rootThread = -1; 
        }
        m_threadCount = threads;
    }


    Scheduler::~Scheduler() {
        if (GetThis() == this) {
            t_scheduler = nullptr;
        }
    }


    Scheduler* Scheduler::GetThis() {
        return t_scheduler;
    }

    Fiber* Scheduler::GetMainFiber() {
        return t_scheduler_fiber;
    }


    void Scheduler::start() {
        LOCK(lock_guard, m_mutex);
        if (!m_stopping) {
            return;
        }
        m_stopping = false;
        SATURN_ASSERT(m_threads.empty());

        m_threads.resize(m_threadCount);
        for(size_t i = 0; i < m_threadCount; ++i) {
            m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this)
                                , m_name + "_" + std::to_string(i)));
            m_threadIds.push_back(m_threads[i]->getId());
        }
    }

    void Scheduler::stop() {
        using enum Fiber::State;
        m_autoStop = true;
        if (m_rootFiber
            && m_threadCount == 0
            && (m_rootFiber->getState() == TERM
            || m_rootFiber->getState() == INIT)) {
            SATURN_LOG_INFO(g_logger) << this << " stopped";
            m_stopping = true;

            if(stopping()) {
                return;
            }
        }
        if (m_rootThread != -1) {
            SATURN_ASSERT(GetThis() == this);
        } else {
            SATURN_ASSERT(GetThis() != this);
        }
        m_stopping = true;
        for(size_t i = 0; i < m_threadCount; ++i) {
            tickle();
        }
    
        if (m_rootFiber) {
            tickle();
        }
        if (m_rootFiber) {
            if (!stopping()) {
                m_rootFiber->call();
            }
        }
        
        std::vector<Thread::ptr> thrs;
        {
            LOCK(lock_guard, m_mutex);
            thrs.swap(m_threads);
        }

        for(auto& i : thrs) {
            i->join();
        }
    }

    void Scheduler::setThis() {
        t_scheduler = this;
    }

    void Scheduler::run() {
        SATURN_LOG_INFO(g_logger) << m_name << " run";
        setThis();
        if (getThreadId() != m_rootThread) {
            t_scheduler_fiber = Fiber::GetThis().get();
        }
        Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
        Fiber::ptr cb_fiber;
        
        FiberAndThread ft;

        while (true) {
            ft.reset();
            bool tickle_me = false;
            bool is_active = false;
            
            {
                LOCK(lock_guard, m_mutex);
                auto it = m_fibers.begin();
                while (it != m_fibers.end()) {
                    if (it->thread != -1 && it->thread != getThreadId()) {
                        tickle_me = true;
                        ++it;
                        continue;
                    }

                    SATURN_ASSERT(it->fiber || it->cb);

                    if (it->fiber  && it->fiber->getState() == Fiber::State::EXEC) {
                        ++it;
                        continue;
                    }

                    ft = *it;
                    m_fibers.erase(it++);
                    m_activeThreadCount++;
                    is_active = true;
                    break;
                }
                tickle_me |= it != m_fibers.end();
            }

            if(tickle_me) {
                tickle();
            }
            
            if(ft.fiber && (ft.fiber->getState() != Fiber::State::TERM
            && ft.fiber->getState() != Fiber::State::EXCEPT)) {
                ft.fiber->swapIn();
                --m_activeThreadCount;
                if(ft.fiber->getState() == Fiber::State::READY) {
                    schedule(ft.fiber);
                } else if(ft.fiber->getState() != Fiber::State::TERM
                        && ft.fiber->getState() != Fiber::State::EXCEPT) {
                    ft.fiber->m_state = Fiber::State::HOLD;
                }
                ft.reset();
            } else if (ft.cb) {
                if(cb_fiber) {
                    cb_fiber->reset(ft.cb);
                } else {
                    cb_fiber.reset(new Fiber(ft.cb));
                }
                ft.reset();
                cb_fiber->swapIn();
                --m_activeThreadCount;
                if(cb_fiber->getState() == Fiber::State::READY) {
                    schedule(cb_fiber);
                    cb_fiber.reset();
                } else if(cb_fiber->getState() == Fiber::State::EXCEPT
                        || cb_fiber->getState() == Fiber::State::TERM) {
                    cb_fiber->reset(nullptr);
                } else {
                    cb_fiber->m_state = Fiber::State::HOLD;
                    cb_fiber.reset();
                }
            }else {
                if(is_active) {
                    --m_activeThreadCount;
                    continue;
                }
                if(idle_fiber->getState() == Fiber::State::TERM) {
                    SATURN_LOG_INFO(g_logger) << "idle fiber term";
                    break;
                }
    
                ++m_idleThreadCount;
                idle_fiber->swapIn();
                --m_idleThreadCount;
                if(idle_fiber->getState() != Fiber::State::TERM
                        && idle_fiber->getState() != Fiber::State::EXCEPT) {
                    idle_fiber->m_state = Fiber::State::HOLD;
                }
            }
            
        }
    }

    void Scheduler::tickle() {
        SATURN_LOG_INFO(g_logger) << "tickle";
    }

    bool Scheduler::stopping() {
        LOCK(lock_guard, m_mutex);
        return m_autoStop && m_stopping
            && m_fibers.empty() && m_activeThreadCount == 0;
    }
    
    void Scheduler::idle() {
        SATURN_LOG_INFO(g_logger) << "idle";
        while(!stopping()) {
            Fiber::YieldToHold();
        }
    }
    void Scheduler::switchTo(int thread) {
        SATURN_ASSERT(Scheduler::GetThis() != nullptr);
        if(Scheduler::GetThis() == this) {
            if(thread == -1 || thread == getThreadId()) {
                return;
            }
        }
        schedule(Fiber::GetThis(), thread);
        Fiber::YieldToHold();
    }
    
    std::ostream& Scheduler::dump(std::ostream& os) {
        os << "[Scheduler name=" << m_name
           << " size=" << m_threadCount
           << " active_count=" << m_activeThreadCount
           << " idle_count=" << m_idleThreadCount
           << " stopping=" << m_stopping
           << " ]" << std::endl << "    ";
        for(size_t i = 0; i < m_threadIds.size(); ++i) {
            if(i) {
                os << ", ";
            }
            os << m_threadIds[i];
        }
        return os;
    }
}
