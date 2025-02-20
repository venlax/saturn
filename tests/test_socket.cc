#include "socket.h"
#include "iomanager.h"
#include "log.h"
#include "macro.h"
#include "util.h"
#include <chrono>

static saturn::Logger::ptr g_looger = LOGGER();

void test_socket() {
    //std::vector<saturn::Address::ptr> addrs;
    //saturn::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    //saturn::IPAddress::ptr addr;
    //for(auto& i : addrs) {
    //    SATURN_LOG_INFO(g_looger) << i->toString();
    //    addr = std::dynamic_pointer_cast<saturn::IPAddress>(i);
    //    if(addr) {
    //        break;
    //    }
    //}
    saturn::IPAddress::ptr addr = saturn::Address::LookupAnyIPAddress("www.baidu.com");
    if(addr) {
        SATURN_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        SATURN_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    saturn::Socket::ptr sock = saturn::Socket::CreateTCP(addr);
    addr->setPort(80);
    SATURN_LOG_INFO(g_looger) << "addr=" << addr->toString();
    if(!sock->connect(addr, 100000)) {
        SATURN_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        SATURN_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0) {
        SATURN_LOG_INFO(g_looger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if(rt <= 0) {
        SATURN_LOG_INFO(g_looger) << "recv fail rt=" << rt;
        return;
    }

    buffs.resize(rt);
    SATURN_LOG_INFO(g_looger) << buffs;
}

void test2() {
    saturn::IPAddress::ptr addr = saturn::Address::LookupAnyIPAddress("www.baidu.com:80");
    if(addr) {
        SATURN_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        SATURN_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    saturn::Socket::ptr sock = saturn::Socket::CreateTCP(addr);
    if(!sock->connect(addr)) {
        SATURN_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        SATURN_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    uint64_t ts = saturn::getCurrentTime<std::chrono::microseconds>();// saturn::GetCurrentUS();
    for(size_t i = 0; i < 10000000000ul; ++i) {
        if(int err = sock->getError()) {
            SATURN_LOG_INFO(g_looger) << "err=" << err << " errstr=" << strerror(err);
            break;
        }

        //struct tcp_info tcp_info;
        //if(!sock->getOption(IPPROTO_TCP, TCP_INFO, tcp_info)) {
        //    SATURN_LOG_INFO(g_looger) << "err";
        //    break;
        //}
        //if(tcp_info.tcpi_state != TCP_ESTABLISHED) {
        //    SATURN_LOG_INFO(g_looger)
        //            << " state=" << (int)tcp_info.tcpi_state;
        //    break;
        //}
        static int batch = 10000000;
        if(i && (i % batch) == 0) {
            uint64_t ts2 = saturn::getCurrentTime<std::chrono::microseconds>();
            SATURN_LOG_INFO(g_looger) << "i=" << i << " used: " << ((ts2 - ts) * 1.0 / batch) << " us";
            ts = ts2;
        }
    }
}

int main(int argc, char** argv) {
    saturn::IOManager iom;
    iom.schedule(&test_socket);
    //iom.schedule(&test2);
    return 0;
}