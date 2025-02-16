#include "address.h"

namespace saturn {
    Address::~Address() {}

    int Address::getFamily() const {}
    
    std::ostream& Address::insert(std::ostream& os) const {}    
    std::string Address::toString() {}

    bool Address::operator<(const Address& rhs) const {}
    bool Address::operator==(const Address& rhs) const {}
    bool Address::operator!=(const Address& rhs) const {}

    IPv4Address::IPv4Address(uint32_t address, uint32_t port) {}

    const sockaddr* IPv4Address::getAddr() const {}
    socklen_t IPv4Address::getAddrLen() const {}
    std::ostream& IPv4Address::insert(std::ostream& os) const {}

    IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len) {}
    IPAddress::ptr IPv4Address::networdAddress(uint32_t prefix_len) {}
    IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len) {}
    uint32_t IPv4Address::getPort() const {}
    void IPv4Address::setPort(uint32_t v) {} 

    IPv6Address::IPv6Address(uint32_t address, uint32_t port) {}

    const sockaddr* IPv6Address::getAddr() const {}
    socklen_t IPv6Address::getAddrLen() const {}
    std::ostream& IPv6Address::insert(std::ostream& os) const {}

    IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len)  {}
    IPAddress::ptr IPv6Address::networdAddress(uint32_t prefix_len) {}
    IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len) {}
    uint32_t IPv6Address::getPort() const {}
    void IPv6Address::setPort(uint32_t v) {} 

    UnixAddress::UnixAddress(const std::string& path) {}

    const sockaddr* UnixAddress::getAddr() const {}
    socklen_t UnixAddress::getAddrLen() const {}
    std::ostream& UnixAddress::insert(std::ostream& os) const {}
    
    UnknownAddress::UnknownAddress() {}
    const sockaddr* UnknownAddress::getAddr() const {}
    socklen_t UnknownAddress::getAddrLen() const {}
    std::ostream& UnknownAddress::insert(std::ostream& os) const {}

    
}