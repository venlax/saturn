#ifndef __SATURN_TCP_SERVER_H__
#define __SATURN_TCP_SERVER_H__

#include <memory>
#include <functional>
#include "address.h"
#include "iomanager.h"
#include "socket.h"

namespace saturn {

    class TcpServer : public std::enable_shared_from_this<TcpServer> {
    public:
        using ptr = std::shared_ptr<TcpServer>;
        TcpServer(saturn::IOManager* woker = saturn::IOManager::GetThis()
                ,saturn::IOManager* accept_woker = saturn::IOManager::GetThis());
        virtual ~TcpServer();

        virtual bool bind(saturn::Address::ptr addr);
        virtual bool bind(const std::vector<Address::ptr>& addrs
                            ,std::vector<Address::ptr>& fails);
        virtual bool start();
        virtual void stop();

        uint64_t getRecvTimeout() const { return m_recvTimeout;}
        std::string getName() const { return m_name;}
        void setRecvTimeout(uint64_t v) { m_recvTimeout = v;}
        void setName(const std::string& v) { m_name = v;}

        bool isStop() const { return m_isStop;}
    protected:
        virtual void handleClient(Socket::ptr client);
        virtual void startAccept(Socket::ptr sock);
    private:
        std::vector<Socket::ptr> m_socks;
        IOManager* m_worker;
        IOManager* m_acceptWorker;
        uint64_t m_recvTimeout;
        std::string m_name;
        bool m_isStop;
    };

}

#endif