#include "thread.h"

#include <mutex>

#include <unistd.h>


#include "config.h"
#include "log.h"
#include "util.h"

using namespace saturn;

Logger::ptr g_logger = LOGGER();

int count = 0;
//RWMutex s_mutex;
std::mutex mutex;
void fun1() {
    SATURN_LOG_INFO(g_logger) << "name: " << Thread::GetName()
                             << " this.name: " << Thread::GetThis()->getName()
                             << " id: " << getThreadId()
                             << " this.id: " << Thread::GetThis()->getId();

    for(int i = 0; i < 100000; ++i) {
        //RWMutex::WriteLock lock(s_mutex);
        std::lock_guard<std::mutex> lock(mutex);
        ++count;
    }
}

void fun2() {
    while(true) {
        SATURN_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void fun3() {
    while(true) {
        SATURN_LOG_INFO(g_logger) << "========================================";
    }
}

int main(int argc, char** argv) {
    SATURN_LOG_INFO(g_logger) << "thread test begin";
    // YAML::Node root = YAML::LoadFile("/home/sylar/test/sylar/bin/conf/log2.yml");
    // Config::loadFromYaml(root);

    std::vector<Thread::ptr> thrs;
    for(int i = 0; i < 2; ++i) {
        Thread::ptr thr(new Thread(&fun2, "name_" + std::to_string(i * 2)));
        Thread::ptr thr2(new Thread(&fun3, "name_" + std::to_string(i * 2 + 1)));
        thrs.push_back(thr);
        thrs.push_back(thr2);
    }

    for(size_t i = 0; i < thrs.size(); ++i) {
        thrs[i]->join();
    }
    SATURN_LOG_INFO(g_logger) << "thread test end";
    SATURN_LOG_INFO(g_logger) << "count=" << count;

    return 0;
}