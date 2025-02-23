#include "http/http_server.h"
#include "log.h"
#include "util.h"

static saturn::Logger::ptr g_logger = LOGGER();

void run() {
    saturn::http::HttpServer::ptr server(new saturn::http::HttpServer);
    saturn::Address::ptr addr = saturn::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/path/xx", [](saturn::http::HttpRequest::ptr req
                ,saturn::http::HttpResponse::ptr rsp
                ,saturn::http::HttpSession::ptr session) {
            std::cout << req->toString() << std::endl;
            rsp->setBody(req->toString());
            return 0;
    });

    sd->addGlobServlet("/path/*", [](saturn::http::HttpRequest::ptr req
                ,saturn::http::HttpResponse::ptr rsp
                ,saturn::http::HttpSession::ptr session) {
            rsp->setBody("Glob:\r\n" + req->toString());
            return 0;
    });
    server->start();
}

int main(int argc, char** argv) {
    saturn::IOManager iom(2);
    iom.schedule(run);
    return 0;
}