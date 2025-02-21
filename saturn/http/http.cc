#include "http.h"
#include <strings.h>

namespace saturn {
namespace http {
    HttpMethod StringToHttpMethod(const std::string& m) {
        #define XX(num, name, string) \
            if(strcmp(#string, m.c_str()) == 0) { \
                return HttpMethod::name; \
            }
        HTTP_METHOD_MAP(XX);
        #undef XX
        return HttpMethod::INVALID_METHOD;
    }

    HttpMethod CharsToHttpMethod(const char* m) {
        #define XX(num, name, string) \
        if(strncmp(#string, m, strlen(#string)) == 0) { \
            return HttpMethod::name; \
        }
        HTTP_METHOD_MAP(XX);
        #undef XX
        return HttpMethod::INVALID_METHOD;
    }

    static const char* s_method_string[] = {
        #define XX(num, name, string) #string,
            HTTP_METHOD_MAP(XX)
        #undef XX
    };

    const char* HttpMethodToString(const HttpMethod& m) {
        uint32_t idx = static_cast<uint32_t>(m);
        return idx >= (sizeof(s_method_string) / sizeof(s_method_string[0])) \
            ? "<unknown>" : s_method_string[idx];
    }

    const char* HttpStatusToString(const HttpStatus& s) {
        switch(s) {
        #define XX(code, name, msg) \
                case HttpStatus::name: \
                    return #msg;
            HTTP_STATUS_MAP(XX);
        #undef XX
            default:
                return "<unknown>";
        }
    }

    bool CaseInsensitiveLess::operator()(const std::string& lhs, const std::string& rhs) const {
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
    }

    HttpRequest::HttpRequest(uint8_t version, bool close)
    :m_method(HttpMethod::GET)
    ,m_version(version)
    ,m_close(close)
    ,m_websocket(false)
    ,m_parserParamFlag(0)
    ,m_path("/") {}

    std::string HttpRequest::getHeader(const std::string& key
        ,const std::string& def) const {
        auto it = m_headers.find(key);
        return it == m_headers.end() ? def : it->second;
    }

    std::shared_ptr<HttpResponse> HttpRequest::createResponse() {
        HttpResponse::ptr rsp(new HttpResponse(getVersion()
                                ,isClose()));
        return rsp;
    }

    std::string HttpRequest::getParam(const std::string& key
        ,const std::string& def) {
        initQueryParam();
        initBodyParam();
        auto it = m_params.find(key);
        return it == m_params.end() ? def : it->second;
    }

    std::string HttpRequest::getCookie(const std::string& key
            ,const std::string& def) {
        initCookies();
        auto it = m_cookies.find(key);
        return it == m_cookies.end() ? def : it->second;
    }

    void HttpRequest::setHeader(const std::string& key, const std::string& val) {
        m_headers[key] = val;
    }
    
    void HttpRequest::setParam(const std::string& key, const std::string& val) {
        m_params[key] = val;
    }
    
    void HttpRequest::setCookie(const std::string& key, const std::string& val) {
        m_cookies[key] = val;
    }
    
    void HttpRequest::delHeader(const std::string& key) {
        m_headers.erase(key);
    }
    
    void HttpRequest::delParam(const std::string& key) {
        m_params.erase(key);
    }
    
    void HttpRequest::delCookie(const std::string& key) {
        m_cookies.erase(key);
    }
    
    bool HttpRequest::hasHeader(const std::string& key, std::string* val) {
        auto it = m_headers.find(key);
        if(it == m_headers.end()) {
            return false;
        }
        if(val) {
            *val = it->second;
        }
        return true;
    }
    
    bool HttpRequest::hasParam(const std::string& key, std::string* val) {
        initQueryParam();
        initBodyParam();
        auto it = m_params.find(key);
        if(it == m_params.end()) {
            return false;
        }
        if(val) {
            *val = it->second;
        }
        return true;
    }
    bool HttpRequest::hasCookie(const std::string& key, std::string* val) {
        initCookies();
        auto it = m_cookies.find(key);
        if(it == m_cookies.end()) {
            return false;
        }
        if(val) {
            *val = it->second;
        }
        return true;
    }
    
    std::string HttpRequest::toString() const {
        std::stringstream ss;
        dump(ss);
        return ss.str();
    }


}    
}