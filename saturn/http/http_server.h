#ifndef __SATURN_HTTP_SERVER_H__
#define __SATURN_HTTP_SERVER_H__

#include "tcpserver.h"
#include "http_session.h"
#include "servlet.h"

namespace saturn {
namespace http {
    class HttpServer : public TcpServer {
    public:
        /// 智能指针类型
        using ptr = std::shared_ptr<HttpServer>;
    
                    HttpServer(bool keepalive = false
                    ,IOManager* worker = IOManager::GetThis()
                    ,IOManager* accept_worker = IOManager::GetThis());
    
                    ServletDispatch::ptr getServletDispatch() const { return m_dispatch;}
    
                    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v;}
        
    protected:
        virtual void handleClient(Socket::ptr client) override;
    private:
        /// 是否支持长连接
        bool m_isKeepalive;
        /// Servlet分发器
        ServletDispatch::ptr m_dispatch;
    };
}
}


#endif // !__SATURN_HTTP_SERVER_H__