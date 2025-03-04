#include <iostream>
#include "http/http_connection.h"
#include "log.h"
#include "iomanager.h"
#include <fstream>
#include "util.h"

static saturn::Logger::ptr g_logger = LOGGER();

void test_pool() {
    saturn::http::HttpConnectionPool::ptr pool(new saturn::http::HttpConnectionPool(
                "httpforever.com", "", 80, 10, 1000 * 30, 5));

    saturn::IOManager::GetThis()->addTimer(1000, [pool](){
            auto r = pool->doGet("/", 3000);
            SATURN_LOG_INFO(g_logger) << r->toString();
    }, true);
}

void run() {
    saturn::Address::ptr addr = saturn::Address::LookupAnyIPAddress("httpforever.com:80");
    if(!addr) {
        SATURN_LOG_INFO(g_logger) << "get addr error";
        return;
    }

    saturn::Socket::ptr sock = saturn::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if(!rt) {
        SATURN_LOG_INFO(g_logger) << "connect " << *addr << " failed";
        return;
    }

    saturn::http::HttpConnection::ptr conn(new saturn::http::HttpConnection(sock));
    saturn::http::HttpRequest::ptr req(new saturn::http::HttpRequest);
    req->setPath("/");
    req->setHeader("host", "httpforever.com");
    SATURN_LOG_INFO(g_logger) << "req:" << std::endl
        << *req;

    conn->sendRequest(req);
    auto rsp = conn->recvResponse();

    if(!rsp) {
        SATURN_LOG_INFO(g_logger) << "recv response error";
        return;
    }
    SATURN_LOG_INFO(g_logger) << "rsp:" << std::endl
        << *rsp;

    std::ofstream ofs("rsp.dat");
    ofs << *rsp;

    SATURN_LOG_INFO(g_logger) << "=========================";

    auto r = saturn::http::HttpConnection::DoGet("http://httpforever.com/", 3000);
    SATURN_LOG_INFO(g_logger) << "result=" << r->result
        << " error=" << r->error
        << " rsp=" << (r->response ? r->response->toString() : "");

    SATURN_LOG_INFO(g_logger) << "=========================";
    test_pool();
}

int main(int argc, char** argv) {
    saturn::IOManager iom(2);
    iom.schedule(run);
    return 0;
}