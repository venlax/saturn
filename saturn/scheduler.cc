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
            m_rootThread = 0; // main thread
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
        if(m_rootFiber
            && m_threadCount == 0
            && (m_rootFiber->getState() == TERM
                || m_rootFiber->getState() == INIT)) {
            SATURN_LOG_INFO(g_logger) << this << " stopped";
            m_stopping = true;

            if(stopping()) {
                return;
            }
        }
    }
}
