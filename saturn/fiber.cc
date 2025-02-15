#include "fiber.h"

#include "config.h"
#include "macro.h"
#include "scheduler.h"
#include "util.h"
#include <ucontext.h>

namespace saturn {
    static Logger::ptr g_logger = LOGGER("system");

    static std::atomic<uint64_t> s_fiber_id {1}; // begin with 1, because 0 is the main fiber
    static std::atomic<uint64_t> s_fiber_count {0};

    static thread_local Fiber* t_fiber = nullptr;
    static thread_local Fiber::ptr t_threadFiber = nullptr;

    static ConfigVar<uint32_t>::ptr g_fiber_stack_size =
    Config::add<uint32_t>("fiber.stack_size", 128 * 1024, "fiber stack size");
    
    Fiber::Fiber() {
        m_state = State::EXEC;
        SetThis(this);
    
        if(getcontext(&m_ctx)) {
            SATURN_ASSERT(false);
        }
    
        ++s_fiber_count;
    
        SATURN_LOG_INFO(g_logger) << "Fiber::Fiber main id=" << m_id;
    }

    Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller) 
    : m_cb(cb), m_id(s_fiber_id++) {
        ++s_fiber_count;
        m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();
    
        m_stack = ::malloc(m_stacksize);
        if(getcontext(&m_ctx)) {
            SATURN_ASSERT(false);
        }
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;
    
        if (!use_caller)
            makecontext(&m_ctx, &Fiber::MainFunc, 0);
        else
            makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    
        SATURN_LOG_INFO(g_logger) << "Fiber::Fiber id=" << m_id;
    }
    Fiber::~Fiber() {
        using enum State;
        --s_fiber_count;
        if(m_stack) {
            SATURN_ASSERT(m_state == TERM
                    || m_state == EXCEPT
                    || m_state == INIT);
    
            ::free(m_stack);
        } else {
            SATURN_ASSERT(!m_cb);
            SATURN_ASSERT(m_state == EXEC);
    
            Fiber* cur = t_fiber;
            if(cur == this) {
                SetThis(nullptr);
            }
        }
        SATURN_LOG_INFO(g_logger) << "Fiber::~Fiber id=" << m_id
                                  << " total=" << s_fiber_count;
    }

    void Fiber::reset(std::function<void()> cb) {
        using enum State;
        SATURN_ASSERT(m_stack);
        SATURN_ASSERT(m_state == TERM
                || m_state == EXCEPT
                || m_state == INIT);
        m_cb = cb;
        if(getcontext(&m_ctx)) {
            SATURN_ASSERT(false);
        }
    
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;
    
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
        m_state = INIT;
    }

    void Fiber::swapIn() {
        SetThis(this);
        SATURN_ASSERT(m_state != State::EXEC);
        this->m_state = State::EXEC;
        if (swapcontext(&Scheduler::GetMainFiber()->m_ctx , &this->m_ctx)) {
            SATURN_ASSERT(false);
        }
    }

    void Fiber::swapOut() {
        SetThis(Scheduler::GetMainFiber());

        if(swapcontext(&this->m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
            SATURN_ASSERT(false);
        }
    }

    void Fiber::call() {
        SetThis(this);
        m_state = State::EXEC;
        if(swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
            SATURN_ASSERT(false);
        }
    }

    void Fiber::back() {
        SetThis(t_threadFiber.get());
        if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
            SATURN_ASSERT(false);
        }
    }

    void Fiber::SetThis(Fiber* f) {
        t_fiber = f;
    }

    Fiber::ptr Fiber::GetThis() {
        if(t_fiber) {
            return t_fiber->shared_from_this();
        }
        // if there is no fiber, we need to construct a fiber to be the main fiber
        Fiber::ptr main_fiber(new Fiber);
        SATURN_ASSERT(t_fiber == main_fiber.get());
        t_threadFiber = main_fiber;
        return t_fiber->shared_from_this();
    }

    void Fiber::YieldToReady() {
        Fiber::ptr cur = GetThis();
        cur->m_state = State::READY;
        cur->swapOut();
    }
    void Fiber::YieldToHold() {
        Fiber::ptr cur = GetThis();
        cur->m_state = State::HOLD;
        cur->swapOut();
    }
    void Fiber::MainFunc() {
        Fiber::ptr cur = GetThis();
        SATURN_ASSERT(cur);
        try {
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = State::TERM;
        } catch (std::exception& ex) {
            cur->m_state = State::EXCEPT;
            SATURN_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
                << " fiber_id=" << cur->getId()
                << std::endl
                << backtraceStr();
        } catch (...) {
            cur->m_state = State::EXCEPT;
            SATURN_LOG_ERROR(g_logger) << "Fiber Except"
                << " fiber_id=" << cur->getId()
                << std::endl
                << backtraceStr();
        }
    
        auto raw_ptr = cur.get();
        cur.reset();
        raw_ptr->swapOut();
    
        SATURN_ASSERT(false);
    }

    void Fiber::CallerMainFunc() {
        using enum State;
        Fiber::ptr cur = GetThis();
        SATURN_ASSERT(cur);
        try {
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = TERM;
        } catch (std::exception& ex) {
            cur->m_state = EXCEPT;
            SATURN_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
                << " fiber_id=" << cur->getId()
                << std::endl
                << saturn::backtraceStr();
        } catch (...) {
            cur->m_state = EXCEPT;
            SATURN_LOG_ERROR(g_logger) << "Fiber Except"
                << " fiber_id=" << cur->getId()
                << std::endl
                << saturn::backtraceStr();
        }
    
        auto raw_ptr = cur.get();
        cur.reset();
        raw_ptr->back();
        SATURN_ASSERT(false);
    
    }

    uint32_t Fiber::GetFiberId() {
        if(t_fiber) {
            return t_fiber->getId();
        }
        return 0;
    }
}