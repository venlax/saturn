#ifndef __SATURN_TIMER_H__
#define __SATURN_TIMER_H__

#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>
namespace saturn {
    class TimerManager;

    class Timer : public std::enable_shared_from_this<Timer> {
    friend class TimerManager;
    public:
        using ptr = std::shared_ptr<Timer>;
        bool cancle();
        bool refresh();
        bool reset(uint64_t ms, bool from_now);
    private:
        Timer(uint64_t ms, std::function<void()> cb,
        bool recurring, TimerManager* manager);
        Timer(uint64_t next);

        struct Comparator {
            bool operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const;
        };

        bool m_recurring = false;
        uint64_t m_ms = 0;
        uint64_t m_next = 0;
        std::function<void()> m_cb;
        TimerManager* m_manager = nullptr;
    };

    class TimerManager {
    friend class Timer;
    public:
        TimerManager();
        virtual ~TimerManager();
        Timer::ptr addTimer(uint64_t ms, std::function<void()> cb
                            ,bool recurring = false);
        Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb
                            ,std::weak_ptr<void> weak_cond
                            ,bool recurring = false);
        uint64_t getNextTimer();
        void listExpiredCb(std::vector<std::function<void()> >& cbs);
        bool hasTimer();
    protected:
        virtual void onTimerInsertedAtFront() = 0;
        void addTimer(Timer::ptr val);
    private:
        std::shared_mutex m_mutex;
        std::set<Timer::ptr, Timer::Comparator> m_timers;
        bool m_tickled = false;
        uint64_t m_previousTime = 0;
    };
}

#endif // !__SATURN_TIMER_H__