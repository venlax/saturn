#include "timer.h"
#include "macro.h"
#include "util.h"
#include <algorithm>
#include <chrono>
#include <memory>
#include <mutex>

namespace saturn {
    using namespace std::chrono;

    bool Timer::Comparator::operator()(const Timer::ptr &lhs, const Timer::ptr &rhs) const {
        if(!lhs && !rhs) {
            return false;
        }
        if(!lhs) {
            return true;
        }
        if(!rhs) {
            return false;
        }
        if(lhs->m_next < rhs->m_next) {
            return true;
        }
        if(rhs->m_next < lhs->m_next) {
            return false;
        }
        return lhs.get() < rhs.get();
    }

    Timer::Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimerManager* manager) 
        : m_ms(ms), m_cb(cb), m_recurring(recurring), m_manager(manager) {
        m_next = getCurrentTime<milliseconds>() + ms;
    }
    Timer::Timer(uint64_t next) : m_next(next) {}

    bool Timer::cancle() {
        LOCK(unique_lock, m_manager->m_mutex);
        if (m_cb) {
            m_cb = nullptr;
            auto it = m_manager->m_timers.find(shared_from_this());
            if (it == m_manager->m_timers.end())
                return false; 
            m_manager->m_timers.erase(it);
            return true;
        }
        return false;
    }
    bool Timer::refresh() {
        LOCK(unique_lock, m_manager->m_mutex);
        if (!m_cb) 
            return false;
        auto it = m_manager->m_timers.find(shared_from_this());
        if (it == m_manager->m_timers.end())
            return false;
        m_manager->m_timers.erase(it);
        m_next = getCurrentTime<milliseconds>() + m_ms;
        m_manager->m_timers.insert(shared_from_this());
        return true;

    }
    bool Timer::reset(uint64_t ms, bool from_now) {
        {
            LOCK(unique_lock, m_manager->m_mutex);
            if (!m_cb) 
                return false;
            auto it = m_manager->m_timers.find(shared_from_this());
            if (it == m_manager->m_timers.end())
                return false;
            m_manager->m_timers.erase(it);
            if (from_now)
                m_next = getCurrentTime<milliseconds>() + m_ms;
            else 
                m_next = m_next - m_ms + ms;
            m_ms = ms;
        }
        m_manager->addTimer(shared_from_this());
        return true;
    }

    TimerManager::TimerManager() {
        m_previousTime = getCurrentTime<milliseconds>();
    }
    TimerManager::~TimerManager() {}

    Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb ,bool recurring) {
        Timer::ptr new_timer {new Timer(ms, cb, recurring, this)};
        addTimer(new_timer);
        return new_timer;
    }

    
    Timer::ptr TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb
        ,std::weak_ptr<void> weak_cond ,bool recurring) {
        auto on_timer = [](std::weak_ptr<void> weak_cond, std::function<void()> cb) {
            auto ptr = weak_cond.lock();
            if (ptr) {
                cb();
            }
        };
        return addTimer(ms, std::bind(on_timer, weak_cond, cb), recurring);
    }
    uint64_t TimerManager::getNextTimer() {
        LOCK(shared_lock, m_mutex);
        m_tickled = false;
        if (m_timers.empty()) 
            return ~0uLL;
        auto& it = *m_timers.begin();
        uint64_t cur = getCurrentTime<milliseconds>();
        if (cur < it->m_next) {
            return it->m_next - cur;
        } 
        return 0;
    }
    void TimerManager::listExpiredCb(std::vector<std::function<void()> >& cbs) {    
        LOCK(shared_lock, m_mutex);
        if (m_timers.empty())
            return;
        std::vector<Timer::ptr> expired;
        Timer::ptr cur {new Timer(getCurrentTime<milliseconds>())};
        auto it = m_timers.upper_bound(cur);
        
        expired.insert(expired.begin(), m_timers.begin(), it);
        m_timers.erase(m_timers.begin(), it);
        UNLOCK();
        {
            LOCK(unique_lock, m_mutex);
            for (auto& ex : expired) {
                cbs.push_back(ex->m_cb);
                // must use the origin timer, can't reconstruct a new one or will cause seg fault (unknown reason)
                if (ex->m_recurring) {
                    ex->m_next = cur->m_next + ex->m_ms;
                    m_timers.insert(ex);
                }
            }
        }

    }
    bool TimerManager::hasTimer() {
        LOCK(shared_lock, m_mutex);
        return !m_timers.empty();
    }
    void TimerManager::addTimer(Timer::ptr val) {
        LOCK(unique_lock, m_mutex);
        auto it = m_timers.insert(val).first;
        bool is_front = it == m_timers.begin() && !m_tickled;
        if (is_front) {
            m_tickled = true;
        }
        UNLOCK();
        if (is_front) {
            onTimerInsertedAtFront();
        }
    }
}