#ifndef __RADIXUN_SOCKET_H__
#define __RADIXUN_SOCKET_H__

#include <memory>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "address.h"
#include "../common/noncopyable.h"

namespace radixun {

//Socket封装类
class Socket : public std::enable_shared_from_this<Socket>, Noncopyable {
public:
    typedef std::shared_ptr<Socket> ptr;
    typedef std::weak_ptr<Socket> weak_ptr;

    //Socket类型
    enum Type {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };
    //Socket协议簇
    enum Family {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNIX = AF_UNIX,
    };

    Socket(int family, int type, int protocol = 0);
    ~Socket();

    static Socket::ptr CreateTCP(radixun::Address::ptr address);
    static Socket::ptr CreateUDP(radixun::Address::ptr address);
    static Socket::ptr CreateTCPSocket();
    static Socket::ptr CreateUDPSocket();
    static Socket::ptr CreateTCPSocket6();
    static Socket::ptr CreateUDPSocket6();
    static Socket::ptr CreateUnixTCPSocket();
    static Socket::ptr CreateUnixUDPSocket();

    //接收connect链接
    Socket::ptr accept();
    //绑定地址
    bool bind(const Address::ptr addr);
    //连接地址(超时时间)
    bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);
    //监听socket (backlog) 未完成连接队列的最大长度
    bool listen(int backlog = SOMAXCONN);
    //关闭socket
    bool close();

    //io  
    //flag(标志字) return  =0关闭 <0出错
    int send(const void* buffer, size_t length, int flags = 0);
    int send(const iovec* buffers, size_t length, int flags = 0);
    int sendTo(const void* buffer, size_t length, const Address::ptr to, int flags = 0);
    int sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags = 0);
    int recv(void* buffer, size_t length, int flags = 0);
    int recv(iovec* buffers, size_t length, int flags = 0);
    int recvFrom(void* buffer, size_t length, Address::ptr from, int flags = 0);
    int recvFrom(iovec* buffers, size_t length, Address::ptr from, int flags = 0);

    bool getOption(int level, int option, void* result, socklen_t* len);
    template<class T>
    bool getOption(int level, int option, T& result) {
        socklen_t length = sizeof(T);
        return getOption(level, option, &result, &length);
    }
    bool setOption(int level, int option, const void* result, socklen_t len);
    template<class T>
    bool setOption(int level, int option, const T& value) {
        return setOption(level, option, &value, sizeof(T));
    }

    int64_t getSendTimeout();
    void setSendTimeout(int64_t v);
    int64_t getRecvTimeout();
    void setRecvTimeout(int64_t v);

    Address::ptr getRemoteAddress();
    Address::ptr getLocalAddress();
    int getFamily() const { return m_family;}
    int getType() const { return m_type;}
    int getProtocol() const { return m_protocol;}
    bool isConnected() const { return m_isConnected;}
    bool isValid() const;
    int getError();
    std::ostream& dump(std::ostream& os) const;
    int getSocket() const { return m_sock;}

    bool cancelRead();
    bool cancelWrite();
    bool cancelAccept();
    bool cancelAll();
private:
    //初始化socket
    void initSock();
    //创建socket
    void newSock();
    //初始化sock
    bool init(int sock);
private:
    /// socket句柄
    int m_sock;
    /// 协议簇
    int m_family;
    /// 类型
    int m_type;
    /// 协议
    int m_protocol;
    /// 是否连接
    bool m_isConnected;
    /// 本地地址
    Address::ptr m_localAddress;
    /// 远端地址
    Address::ptr m_remoteAddress;
};

//流式输出socket
std::ostream& operator<<(std::ostream& os, const Socket& sock);

}

#endif
