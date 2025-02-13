#include "scheduler.h"
#include "util.h"
#include "log.h"

static saturn::Logger::ptr g_logger = LOGGER();

void test_fiber() {
    static int s_count = 5;
    SATURN_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    sleep(1);
    if(--s_count >= 0) {
        saturn::Scheduler::GetThis()->schedule(&test_fiber, saturn::getThreadId());
    }
}

int main(int argc, char** argv) {
    SATURN_LOG_INFO(g_logger) << "main";
    saturn::Scheduler sc(3, true, "test");
    sc.start();
    sleep(2);
    SATURN_LOG_INFO(g_logger) << "schedule";
    sc.schedule(&test_fiber);
    sc.stop();
    SATURN_LOG_INFO(g_logger) << "over";
    return 0;
}