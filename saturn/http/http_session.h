#ifndef __SATURN_HTTP_SESSION_H__
#define __SATURN_HTTP_SESSION_H__


#include "http.h"
#include "stream/socketstream.h"


namespace saturn {
namespace http {
    class HttpSession : public SocketStream {
        public:
            using ptr =  std::shared_ptr<HttpSession>;
        
            HttpSession(Socket::ptr sock, bool owner = true);
        
            HttpRequest::ptr recvRequest();
        
            int sendResponse(HttpResponse::ptr rsp);
        };
}
}

#endif // !__SATURN_HTTP_SESSION_H__