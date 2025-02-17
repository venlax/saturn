#include "address.h"
#include "log.h"
#include "util.h"

saturn::Logger::ptr g_logger = LOGGER();

void test() {
    std::vector<saturn::Address::ptr> addrs;

    SATURN_LOG_INFO(g_logger) << "begin";
    //bool v = saturn::Address::Lookup(addrs, "localhost:3080");
    bool v = saturn::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    //bool v = saturn::Address::Lookup(addrs, "www.saturn.top", AF_INET);
    SATURN_LOG_INFO(g_logger) << "end";
    if(!v) {
        SATURN_LOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    for(size_t i = 0; i < addrs.size(); ++i) {
        SATURN_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }

    auto addr = saturn::Address::LookupAny("localhost:4080");
    if(addr) {
        SATURN_LOG_INFO(g_logger) << *addr->getAddr()->sa_data;
    } else {
        SATURN_LOG_ERROR(g_logger) << "error";
    }
}

void test_iface() {
    std::multimap<std::string, std::pair<saturn::Address::ptr, uint32_t> > results;

    bool v = saturn::Address::GetInterfaceAddresses(results);
    if(!v) {
        SATURN_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for(auto& i: results) {
        SATURN_LOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << " - "
            << i.second.second;
    }
}

void test_ipv4() {
    //auto addr = saturn::IPAddress::Create("www.saturn.top");
    auto addr = saturn::IPAddress::Create("127.0.0.8");
    if(addr) {
        SATURN_LOG_INFO(g_logger) << addr->toString();
    }
}

int main(int argc, char** argv) {
    //test_ipv4();
    test_iface();
    //test();
    return 0;
}