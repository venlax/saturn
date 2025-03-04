#ifndef __SATURN_URI_H__
#define __SATURN_URI_H__

#include "address.h"
#include <memory>
#include <ostream>
#include <string>
namespace saturn {
    class Uri {
    public:
        using ptr = std::shared_ptr<Uri>;
        Uri();
        static Uri::ptr Create(const std::string& uri);
        const std::string& getScheme() const {return m_scheme;}
        const std::string& getUserinfo() const {return m_userinfo;}
        const std::string& getHost() const {return m_host;}
        const std::string& getPath() const;
        const std::string& getQuery() const {return m_query;}
        const std::string& getFragment() const {return m_fragment;}
        int32_t getPort() const;
        
        void setScheme(const std::string& scheme) {m_scheme = scheme;}
        void setUserinfo(const std::string& userinfo) { m_userinfo = userinfo;}
        void setHost(const std::string& host)  {m_host = host;}
        void setPath(const std::string& path)  { m_path = path;}
        void setQuery(const std::string& query)  { m_query = query;}
        void setFragment(const std::string& fragment) {m_fragment = fragment;}
        void setPort(int32_t port)  { m_port = port;}
        
        std::ostream& dump(std::ostream& os) const;
        std::string toString() const;
        Address::ptr createAddress() const;

        bool isDefaultPort() const;

    private:
        std::string m_scheme;
        std::string m_userinfo;
        std::string m_host;
        std::string m_path;
        std::string m_query;
        std::string m_fragment;
        int32_t m_port;
    };
}



#endif // !__SATURN_URI_H__