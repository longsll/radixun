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

    //创建TCP Socket(满足地址类型)
    static Socket::ptr CreateTCP(radixun::Address::ptr address);
    //创建UDP Socket(满足地址类型)
    static Socket::ptr CreateUDP(radixun::Address::ptr address);
    //创建IPv4的TCP Socket
    static Socket::ptr CreateTCPSocket();
    //创建IPv4的UDP Socket
    static Socket::ptr CreateUDPSocket();
    //创建IPv6的TCP Socket
    static Socket::ptr CreateTCPSocket6();
    //创建IPv6的UDP Socket
    static Socket::ptr CreateUDPSocket6();
    //创建Unix的TCP Socket
    static Socket::ptr CreateUnixTCPSocket();
    //创建Unix的UDP Socket
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

    /**
     * @brief 发送数据
     * @param[in] buffer 待发送数据的内存
     * @param[in] length 待发送数据的长度
     * @param[in] flags 标志字
     * @return
     *      @retval >0 发送成功对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    int send(const void* buffer, size_t length, int flags = 0);
    int send(const iovec* buffers, size_t length, int flags = 0);
    //发送数据
    int sendTo(const void* buffer, size_t length, const Address::ptr to, int flags = 0);
    int sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags = 0);
    //接受数据
    int recv(void* buffer, size_t length, int flags = 0);
    int recv(iovec* buffers, size_t length, int flags = 0);
    ///接受数据
    int recvFrom(void* buffer, size_t length, Address::ptr from, int flags = 0);
    int recvFrom(iovec* buffers, size_t length, Address::ptr from, int flags = 0);

    //获取发送超时时间(毫秒)
    int64_t getSendTimeout();
    //设置发送超时时间(毫秒)
    void setSendTimeout(int64_t v);
    //获取接受超时时间(毫秒)
    int64_t getRecvTimeout();
    //设置接受超时时间(毫秒)
    void setRecvTimeout(int64_t v);
    //获取sockopt
    bool getOption(int level, int option, void* result, socklen_t* len);
    //获取sockopt模板
    template<class T>
    bool getOption(int level, int option, T& result) {
        socklen_t length = sizeof(T);
        return getOption(level, option, &result, &length);
    }
    //设置sockopt
    bool setOption(int level, int option, const void* result, socklen_t len);
    //设置sockopt模板
    template<class T>
    bool setOption(int level, int option, const T& value) {
        return setOption(level, option, &value, sizeof(T));
    }
    //获取远端地址
    Address::ptr getRemoteAddress();
    //获取本地地址
    Address::ptr getLocalAddress();
    //获取协议簇
    int getFamily() const { return m_family;}
    //获取类型
    int getType() const { return m_type;}
    //获取协议
    int getProtocol() const { return m_protocol;}
    //返回是否连接
    bool isConnected() const { return m_isConnected;}
    //是否有效(m_sock != -1)
    bool isValid() const;
    //返回Socket错误
    int getError();
    //输出信息到流中
    std::ostream& dump(std::ostream& os) const;
    //返回socket句柄
    int getSocket() const { return m_sock;}

    //取消读
    bool cancelRead();
    //取消写
    bool cancelWrite();
    //取消accept
    bool cancelAccept();
    //取消所有事件
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
