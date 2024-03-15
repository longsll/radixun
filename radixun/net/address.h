#ifndef __RADIXUN_ADDRESS_H__
#define __RADIXUN_ADDRESS_H__

#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>

namespace radixun{

class IPAddress;

//  网络地址的基类,抽象类
class Address{
public:
    typedef std::shared_ptr<Address> ptr;

    static Address::ptr Create(const sockaddr* addr , socklen_t addrlen);
    //通过host地址返回对应条件的所有Address
    //(type socketl类型SOCK_STREAM、SOCK_DGRAM 等)(协议,IPPROTO_TCP、IPPROTO_UDP 等)
    static bool Lookup(std::vector<Address::ptr>& result , const std::string& host,
                    int family = AF_INET , int type = 0 , int protocol = 0);
    static Address::ptr LookupAny(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);
    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);
    //返回本机所有网卡的<网卡名, 地址, 子网掩码位数>
    static bool GetInterfaceAddresses(std::multimap<std::string
                    ,std::pair<Address::ptr, uint32_t> >& result,
                    int family = AF_INET);
    //获取本机指定网卡的地址和子网掩码位数
    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >&result
                    ,const std::string& iface, int family = AF_INET);
    virtual ~Address() {}
    int getFamily() const;
    // 返回sockaddr指针,只读
    virtual const sockaddr* getAddr() const = 0;
    // 返回sockaddr指针,读写
    virtual sockaddr* getAddr() = 0;
    // 返回sockaddr的长度
    virtual socklen_t getAddrLen() const = 0;
    // 可读性输出地址
    virtual std::ostream& insert(std::ostream& os) const = 0;
    std::string toString() const;
    bool operator<(const Address& rhs) const;
    bool operator==(const Address& rhs) const;
    bool operator!=(const Address& rhs) const;

};

// IP地址的基类
class IPAddress : public Address {
public:
    typedef std::shared_ptr<IPAddress> ptr;

    //通过域名,IP,服务器名创建IPAddress
    static IPAddress::ptr Create(const char* address, uint16_t port = 0);
    //返回广播地址(广播地址是该子网掩码与IP地址进行逻辑“或”操作后得到的结果)
    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
    //返回网络地址(网络地址是通过子网掩码与IP地址进行逻辑与（AND）操作来确定的)
    virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;
    //获取子网掩码地址
    virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;
    virtual uint32_t getPort() const = 0;
    virtual void setPort(uint16_t v) = 0;
};

// IPv4地址
class IPv4Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv4Address> ptr;

    //使用点分十进制地址创建IPv4Address
    static IPv4Address::ptr Create(const char* address, uint16_t port = 0);
    //通过sockaddr_in构造IPv4Address
    IPv4Address(const sockaddr_in& address);
    //通过二进制地址构造IPv4Address
    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;
    uint32_t getPort() const override;
    void setPort(uint16_t v) override;
private:
    sockaddr_in m_addr;
};

// IPv6地址
class IPv6Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv6Address> ptr;


    static IPv6Address::ptr Create(const char* address, uint16_t port = 0);
    IPv6Address();
    IPv6Address(const sockaddr_in6& address);
    IPv6Address(const uint8_t address[16], uint16_t port = 0);

    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;
    uint32_t getPort() const override;
    void setPort(uint16_t v) override;
private:
    sockaddr_in6 m_addr;
};

// UnixSocket地址
class UnixAddress : public Address {
public:
    typedef std::shared_ptr<UnixAddress> ptr;

    UnixAddress();
    //通过路径构造UnixAddress
    UnixAddress(const std::string& path);

    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    void setAddrLen(uint32_t v);
    std::ostream& insert(std::ostream& os) const override;
private:
    sockaddr_un m_addr;
    socklen_t m_length;
};

// 未知地址
class UnknownAddress : public Address {
public:
    typedef std::shared_ptr<UnknownAddress> ptr;
    UnknownAddress(int family);
    UnknownAddress(const sockaddr& addr);
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
private:
    sockaddr m_addr;
};

std::ostream& operator<<(std::ostream& os, const Address& addr);

}


#endif