#ifndef __SATURN_ADDRESS_H__
#define __SATURN_ADDRESS_H__

#include <memory>
#include <ostream>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>


namespace saturn {
    class Address {
    public:
        using ptr = std::shared_ptr<Address>;
        virtual ~Address();

        int getFamily() const;

        virtual const sockaddr* getAddr() const = 0;
        virtual socklen_t getAddrLen() const = 0;
        virtual std::ostream& insert(std::ostream& os) const;
        std::string toString();

        bool operator<(const Address& rhs) const;
        bool operator==(const Address& rhs) const;
        bool operator!=(const Address& rhs) const;
    };

    class IPAddress : public Address {
    public:
        using ptr = std::shared_ptr<IPAddress>;
        virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
        virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;
        virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;
        
        virtual uint32_t getPort() const = 0;
        virtual void setPort(uint32_t v) = 0;
    };

    class IPv4Address : public IPAddress {
    public:
        using ptr = std::shared_ptr<IPv4Address>;

        IPv4Address(uint32_t address = INADDR_ANY, uint32_t port = 0);

        const sockaddr* getAddr() const override;
        socklen_t getAddrLen() const override;
        std::ostream& insert(std::ostream& os) const override;

        IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
        IPAddress::ptr networdAddress(uint32_t prefix_len) override;
        IPAddress::ptr subnetMask(uint32_t prefix_len) override;
        uint32_t getPort() const override;
        void setPort(uint32_t v) override; 
    private:
        sockaddr_in m_addr;
    };

    class IPv6Address : public IPAddress {
    public:
        using ptr = std::shared_ptr<IPv6Address>;

        IPv6Address(uint32_t address = INADDR_ANY, uint32_t port = 0);

        const sockaddr* getAddr() const override;
        socklen_t getAddrLen() const override;
        std::ostream& insert(std::ostream& os) const override;

        IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
        IPAddress::ptr networdAddress(uint32_t prefix_len) override;
        IPAddress::ptr subnetMask(uint32_t prefix_len) override;
        uint32_t getPort() const override;
        void setPort(uint32_t v) override; 
    private:
        sockaddr_in6 m_addr;
    };

    class UnixAddress : public Address {
    public:
        using ptr = std::shared_ptr<UnixAddress>;
        UnixAddress(const std::string& path);

        const sockaddr* getAddr() const override;
        socklen_t getAddrLen() const override;
        std::ostream& insert(std::ostream& os) const override;
    private:
        sockaddr_un m_addr;
        socklen_t m_length;
    };

    class UnknownAddress : public Address {
    public:
        using ptr = std::shared_ptr<UnknownAddress>;
        UnknownAddress();
        const sockaddr* getAddr() const override;
        socklen_t getAddrLen() const override;
        std::ostream& insert(std::ostream& os) const override;
    private:
        sockaddr m_addr;
    };
}
#endif // !__SATURN_ADDRESS_H__