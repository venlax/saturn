#ifndef __SATURN_THREAD_H__
#define __SATURN_THREAD_H__

#include <functional>
#include <memory>
#include <string>

#include <pthread.h>

namespace saturn {
    class Thread {
    private:
        pid_t m_id;
        std::string m_name;
        pthread_t m_thread;
        std::function<void()> m_cb;
    public:
        using ptr = std::shared_ptr<Thread>;
        Thread(std::function<void()> cb, std::string_view name);
        ~Thread();
        std::string_view getName() const {return m_name;};
        pid_t getId() const {return m_id;};
        void join();

        static std::string_view GetName();
        static void SetName(std::string_view name);
        static void* run(void* args);
        static Thread* GetThis();
    };
}


#endif //!__SATURN_THREAD_H__