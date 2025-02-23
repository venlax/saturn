#ifndef __SATURN_CONFIG_SERVLET_H__
#define __SATURN_CONFIG_SERVLET_H__

#include "http/servlet.h"

namespace saturn {
namespace http {

    class ConfigServlet : public Servlet {
    public:
        ConfigServlet();
        virtual int32_t handle(saturn::http::HttpRequest::ptr request
                    , saturn::http::HttpResponse::ptr response
                    , saturn::http::HttpSession::ptr session) override;
    };

}
}

#endif