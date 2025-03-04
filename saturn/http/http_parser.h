#ifndef __SATURN_HTTP_PARSER_H__
#define __SATURN_HTTP_PARSER_H__

#include "http.h"
#include "external/mongrel2/http/http11_parser.h"
#include "external/mongrel2/http/httpclient_parser.h"

namespace saturn {
namespace http {

    class HttpRequestParser {
    public:
        using ptr =  std::shared_ptr<HttpRequestParser>;
        HttpRequestParser();
        size_t execute(char* data, size_t len);
        int isFinished();
        int hasError(); 

        HttpRequest::ptr getData() const { return m_data;}
        void setError(int v) { m_error = v;}
        const http_parser& getParser() const { return m_parser;}
        uint64_t getContentLength();

        static uint64_t GetHttpRequestBufferSize();
        static uint64_t GetHttpRequestMaxBodySize();

    private:
        http_parser m_parser;
        HttpRequest::ptr m_data;
        //1000: invalid method
        //1001: invalid version
        //1002: invalid field
        int m_error;
    };

    class HttpResponseParser {
    public:
        using ptr = std::shared_ptr<HttpResponseParser>;
        HttpResponseParser();
        size_t execute(char* data, size_t len, bool chunk);
        int isFinished();
        int hasError(); 

        HttpResponse::ptr getData() const { return m_data;}
        void setError(int v) { m_error = v;}

        uint64_t getContentLength();

        const httpclient_parser& getParser() const { return m_parser;}

        static uint64_t GetHttpResponseBufferSize();
        static uint64_t GetHttpResponseMaxBodySize();
    private:
        httpclient_parser m_parser;
        HttpResponse::ptr m_data;
        //1001: invalid version
        //1002: invalid field
        int m_error;
    };

}
}

#endif