#include "http/http.h"
#include "log.h"

void test_request() {
    saturn::http::HttpRequest::ptr req(new saturn::http::HttpRequest);
    req->setHeader("host" , "www.saturn.top");
    req->setBody("hello saturn");
    req->dump(std::cout) << std::endl;
}

void test_response() {
    saturn::http::HttpResponse::ptr rsp(new saturn::http::HttpResponse);
    rsp->setHeader("X-X", "saturn");
    rsp->setBody("hello saturn");
    rsp->setStatus((saturn::http::HttpStatus)400);
    rsp->setClose(false);

    rsp->dump(std::cout) << std::endl;
}

int main(int argc, char** argv) {
    test_request();
    test_response();
    return 0;
}