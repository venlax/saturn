#include "thread.h"

#include "util.h"

namespace saturn {
    static thread_local Thread* t_thread = nullptr;
    static thread_local std::string t_thread_name = "UNKNOWN";
    static Logger::ptr g_logger = LOGGER("system");

    std::string_view Thread::GetName() {
        return t_thread_name;
    }

    Thread* Thread::GetThis() {
        return t_thread;
    }
    void Thread::SetName(std::string_view name) {
        if (name.empty()) {
            return;
        }
        t_thread_name = name;
        if (t_thread) t_thread->SetName(name);
    }

    Thread::Thread(std::function<void()> cb , std::string_view name) : m_cb(cb), m_name(name) {
        if (name.empty()) {
            m_name = "UNKNOWN";
        }
        int rt = pthread_create(&m_thread, nullptr  , &Thread::run, this);
        if (rt) {
            SATURN_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt
                << " name=" << name;
            throw std::logic_error("pthread_create error");
        }
    }

    void* Thread::run(void* args) {
        Thread* thread = static_cast<Thread*>(args);
        t_thread = thread;
        t_thread_name = thread->m_name;
        thread->m_id = getThreadId();
        pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());
        std::function<void()> cb;
        cb.swap(thread->m_cb);
        cb();
        return 0;
    }

    void Thread::join() {
        if (m_thread) {
            int rt = pthread_join(m_thread, nullptr);
            if(rt) {
                SATURN_LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt
                    << " name=" << m_name;
                throw std::logic_error("pthread_join error");
            }
            m_thread = 0;
        }
    }

    Thread::~Thread() {
        if (m_thread) {
            pthread_detach(m_thread);
        }
    }

}