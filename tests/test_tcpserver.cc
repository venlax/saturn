#include "tcpserver.h"
#include "iomanager.h"
#include "log.h"
#include "util.h"

saturn::Logger::ptr g_logger = LOGGER();

void run() {
    auto addr = saturn::Address::LookupAny("0.0.0.0:8033");
    //auto addr2 = saturn::UnixAddress::ptr(new saturn::UnixAddress("/tmp/unix_addr"));
    std::vector<saturn::Address::ptr> addrs;
    addrs.push_back(addr);
    //addrs.push_back(addr2);

    saturn::TcpServer::ptr tcp_server(new saturn::TcpServer);
    std::vector<saturn::Address::ptr> fails;
    while(!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();
    
}
int main(int argc, char** argv) {
    saturn::IOManager iom(2);
    iom.schedule(run);
    return 0;
}