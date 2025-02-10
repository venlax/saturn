#include "fiber.h"

#include "config.h"
#include "macro.h"
#include <ucontext.h>

namespace saturn {
    static Logger::ptr g_logger = LOGGER("system");

    static std::atomic<uint64_t> s_fiber_id {1};
    static std::atomic<uint64_t> s_fiber_count {0};

    static thread_local Fiber* t_fiber = nullptr;
    static thread_local Fiber::ptr t_threadFiber = nullptr;

    static ConfigVar<uint32_t>::ptr g_fiber_stack_size =
    Config::add<uint32_t>("fiber.stack_size", 128 * 1024, "fiber stack size");
    
    Fiber::Fiber() {
        m_state = State::EXEC;
        setThis(this);
    
        if(getcontext(&m_ctx)) {
            SATURN_ASSERT(false);
        }
    
        ++s_fiber_count;
    
        SATURN_LOG_DEBUG(g_logger) << "Fiber::Fiber main";
    }

    Fiber::Fiber(std::function<void()> cb, size_t stacksize) 
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
    
        makecontext(&m_ctx, &Fiber::mainFunc, 0);

    
        SATURN_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
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
                setThis(nullptr);
            }
        }
        SATURN_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id
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
    
        makecontext(&m_ctx, &Fiber::mainFunc, 0);
        m_state = INIT;
    }

    void Fiber::swapIn() {
        setThis(this);
        SATURN_ASSERT(m_state != State::EXEC);
        this->m_state = State::EXEC;
        if (swapcontext(&t_threadFiber->m_ctx , &this->m_ctx)) {
            SATURN_ASSERT(false);
        }
    }
    void Fiber::swapOut() {
        setThis(t_threadFiber.get());

        if(swapcontext(&this->m_ctx, &t_threadFiber->m_ctx)) {
            SATURN_ASSERT(false);
        }
    }

    void Fiber::setThis(Fiber* f) {
        t_fiber = f;
    }

    Fiber::ptr Fiber::getThis() {
        if(t_fiber) {
            return t_fiber->shared_from_this();
        }
        Fiber::ptr main_fiber(new Fiber);
        SATURN_ASSERT(t_fiber == main_fiber.get());
        t_threadFiber = main_fiber;
        return t_fiber->shared_from_this();
    }

    void Fiber::yieldToReady() {
        Fiber::ptr cur = getThis();
        cur->m_state = State::READY;
        cur->swapOut();
    }
    void Fiber::yieldToHold() {
        Fiber::ptr cur = getThis();
        cur->m_state = State::HOLD;
        cur->swapOut();
    }
    void Fiber::mainFunc() {
        Fiber::ptr cur = getThis();
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

    uint32_t Fiber::getFiberId() {
        if(t_fiber) {
            return t_fiber->getId();
        }
        return 0;
    }
}