#ifndef __SATURN_SERVLET_H__
#define __SATURN_SERVLET_H__

#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>

#include "http/http.h"
#include "http/http_session.h"


namespace saturn {
namespace http {
    class Servlet {
        public:
            using ptr = std::shared_ptr<Servlet>;
        
            Servlet(const std::string& name)
                :m_name(name) {}
            virtual ~Servlet() {}
            virtual int32_t handle(HttpRequest::ptr request
                           , HttpResponse::ptr response
                           , HttpSession::ptr session) = 0;
                           
            const std::string& getName() const { return m_name;}
        protected:
            std::string m_name;
        };
        
        class FunctionServlet : public Servlet {
        public:
            using ptr =  std::shared_ptr<FunctionServlet>;
            using callback = std::function<int32_t (HttpRequest::ptr request
                           , HttpResponse::ptr response
                           , HttpSession::ptr session)>;
        
        
            FunctionServlet(callback cb);
            virtual int32_t handle(HttpRequest::ptr request
                           , HttpResponse::ptr response
                           , HttpSession::ptr session) override;
        private:
            callback m_cb;
        };
        
        class ServletDispatch : public Servlet {
        public:
            using ptr =  std::shared_ptr<ServletDispatch>;

            ServletDispatch();
            virtual int32_t handle(HttpRequest::ptr request
                           , HttpResponse::ptr response
                           , HttpSession::ptr session) override;
        
            void addServlet(const std::string& uri, Servlet::ptr slt);
            void addServlet(const std::string& uri, FunctionServlet::callback cb);
            void addGlobServlet(const std::string& uri, Servlet::ptr slt);
            void addGlobServlet(const std::string& uri, FunctionServlet::callback cb);
        
            void delServlet(const std::string& uri);
            void delGlobServlet(const std::string& uri);
        
            Servlet::ptr getDefault() const { return m_default;}
            void setDefault(Servlet::ptr v) { m_default = v;}
        
            Servlet::ptr getServlet(const std::string& uri);
            Servlet::ptr getGlobServlet(const std::string& uri);
        
            Servlet::ptr getMatchedServlet(const std::string& uri);
        private:
            std::shared_mutex m_mutex;

            std::unordered_map<std::string, Servlet::ptr> m_datas;
            std::vector<std::pair<std::string, Servlet::ptr> > m_globs;
            Servlet::ptr m_default;
        };
        
        class NotFoundServlet : public Servlet {
        public:
            using ptr = std::shared_ptr<NotFoundServlet>;
            NotFoundServlet();
            virtual int32_t handle(HttpRequest::ptr request
                           , HttpResponse::ptr response
                           , HttpSession::ptr session) override;
        
        };
    }    
}


#endif // !__SATURN_SERVLET_H__