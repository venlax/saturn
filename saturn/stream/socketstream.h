#ifndef __SATURN_SOCKETSTREAM_H__
#define __SATURN_SOCKETSTREAM_H__
#include "iomanager.h"
#include "socket.h"
#include "stream.h"

namespace saturn {
    class SocketStream : public Stream {
        public:
        using ptr = std::shared_ptr<SocketStream>;
    
        SocketStream(Socket::ptr sock, bool owner = true);
    
        ~SocketStream();
    
        virtual int read(void* buffer, size_t length) override;
    
        virtual int read(ByteArray::ptr ba, size_t length) override;
    
        virtual int write(const void* buffer, size_t length) override;
        
        virtual int write(ByteArray::ptr ba, size_t length) override;
    
        virtual void close() override;
        
        Socket::ptr getSocket() const { return m_socket;}
    
        bool isConnected() const;
    
        Address::ptr getRemoteAddress();
        Address::ptr getLocalAddress();
        std::string getRemoteAddressString();
        std::string getLocalAddressString();
    protected:
        Socket::ptr m_socket;
        bool m_owner;
    };
}


#endif // !__SATURN_SOCKETSTREAM_H__


