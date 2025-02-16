#include "hook.h"

#include <dlfcn.h>

#include "fiber.h"
#include "iomanager.h"

namespace saturn {
    static thread_local bool t_hook_enable = false;

    #define HOOK_FUN(OP) \
        OP(sleep) \
        OP(usleep) \
        OP(nanosleep) \
        OP(socket) \
        OP(connect) \
        OP(accept) \
        OP(read) \
        OP(readv) \
        OP(recv) \
        OP(recvfrom) \
        OP(recvmsg) \
        OP(write) \
        OP(writev) \
        OP(send) \
        OP(sendto) \
        OP(sendmsg) \
        OP(close) \
        OP(fcntl) \
        OP(ioctl) \
        OP(getsockopt) \
        OP(setsockopt)
    void hook_init() {
        static bool is_inited = false;
        if(is_inited) {
            return;
        }
    #define OP(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
        HOOK_FUN(OP);
    #undef OP
    }
    struct HookIniter {
        HookIniter() {
            hook_init();
        }
    };

    static HookIniter hook_initer;

    bool is_hook_enable() {return t_hook_enable;}

    void set_hook_enable(bool flag) {t_hook_enable = flag;}
}

extern "C" {
    #define OP(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(OP);
    #undef OP

    unsigned int sleep(unsigned int seconds) {
        if (!saturn::t_hook_enable) {
            return sleep_f(seconds);
        }
        saturn::Fiber::ptr fiber = saturn::Fiber::GetThis();
        saturn::IOManager* iom = saturn::IOManager::GetThis();
        iom->addTimer(seconds * 1000, std::bind((void(saturn::Scheduler::*)
                (saturn::Fiber::ptr, int thread))&saturn::IOManager::schedule
                ,iom, fiber, -1));
        saturn::Fiber::YieldToHold();
        return 0;
    }
}