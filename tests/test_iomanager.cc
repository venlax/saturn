#include "iomanager.h"
#include "log.h"
#include "macro.h"
#include "util.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>

saturn::Logger::ptr g_logger = LOGGER();

int sock = 0;

void test_fiber() {
    SATURN_LOG_INFO(g_logger) << "test_fiber sock=" << sock;

    //sleep(3);

    //close(sock);
    //saturn::IOManager::GetThis()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "10.255.255.254", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        SATURN_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        saturn::IOManager::GetThis()->addEvent(sock, saturn::IOManager::READ, [](){
            SATURN_LOG_INFO(g_logger) << "read callback";
        });
        saturn::IOManager::GetThis()->addEvent(sock, saturn::IOManager::WRITE, [](){
            SATURN_LOG_INFO(g_logger) << "write callback";
            //close(sock);
            saturn::IOManager::GetThis()->cancelEvent(sock, saturn::IOManager::READ);
            close(sock);
            
        });
    } else {
        SATURN_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }

}
void test_fiber2() {
    static int s_count = 5;
    SATURN_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    sleep(1);
    if(--s_count >= 0) {
        saturn::Scheduler::GetThis()->schedule(&test_fiber2, saturn::getThreadId());
    }
}
void test1() {
    saturn::IOManager iom(2, false);
    iom.schedule(&test_fiber);           
}

saturn::Timer::ptr s_timer;
void test_timer() {
    saturn::IOManager iom(2);
    s_timer = iom.addTimer(1000, [](){
        static int i = 0;
        SATURN_LOG_INFO(g_logger) << "hello timer i=" << i;
        if(++i == 3) {
            s_timer->reset(2000, true);
            //s_timer->cancel();
        }
    }, true);
}

int main(int argc, char** argv) {
    test_timer();
    return 0;
}