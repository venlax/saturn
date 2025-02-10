#include "fiber.h"
#include "macro.h"
#include "log.h"
#include "thread.h"
#include "util.h"

saturn::Logger::ptr g_logger = LOGGER();

void run_in_fiber() {
    SATURN_LOG_INFO(g_logger) << "run_in_fiber begin";
    saturn::Fiber::yieldToHold();
    SATURN_LOG_INFO(g_logger) << "run_in_fiber end";
    saturn::Fiber::yieldToHold();
}

void test_fiber() {
    SATURN_LOG_INFO(g_logger) << "main begin -1";
    {
        saturn::Fiber::getThis();
        SATURN_LOG_INFO(g_logger) << "main begin";
        saturn::Fiber::ptr fiber(new saturn::Fiber(run_in_fiber, 0));
        fiber->swapIn();
        SATURN_LOG_INFO(g_logger) << "main after swapIn";
        fiber->swapIn();
        SATURN_LOG_INFO(g_logger) << "main after end";
        fiber->swapIn();
    }
    SATURN_LOG_INFO(g_logger) << "main after end2";
}

int main(int argc, char** argv) {
    saturn::Thread::SetName("main");

    std::vector<saturn::Thread::ptr> thrs;
    for(int i = 0; i < 3; ++i) {
        thrs.push_back(saturn::Thread::ptr(
                    new saturn::Thread(&test_fiber, "name_" + std::to_string(i))));
    }
    for(auto i : thrs) {
        i->join();
    }
    return 0;
}