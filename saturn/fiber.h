#ifndef __SATURN_FIBER_H__
#define __SATURN_FIBER_H__

#include <functional>
#include <memory>
#include <ucontext.h>


namespace saturn
{


    class Fiber : public std::enable_shared_from_this<Fiber>{
    public:
        using ptr = std::shared_ptr<Fiber>;
        enum class State {
            /// 初始化状态
            INIT,
            /// 暂停状态
            HOLD,
            /// 执行中状态
            EXEC,
            /// 结束状态
            TERM,
            /// 可执行状态
            READY,
            /// 异常状态
            EXCEPT
        };
    private:
        uint32_t m_id = 0;
        State m_state;
        size_t m_stacksize;
        ucontext_t m_ctx;
        void* m_stack = nullptr;
        std::function<void()> m_cb;    
    public:
        Fiber();
        Fiber(std::function<void()> cb, size_t stacksize);
        ~Fiber();
        void swapIn();
        void swapOut();
        void reset(std::function<void()> cb);
        uint32_t getId() const {return m_id;}
        State getState() const {return m_state;}

        static void setThis(Fiber* f);
        static Fiber::ptr getThis();
        static void yieldToReady();
        static void yieldToHold();
        static void mainFunc();
        static uint32_t getFiberId();
    };
}


#endif // !__SATURN_FIBER_H__
