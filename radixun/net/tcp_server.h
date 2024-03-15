#ifndef __RADIXUN_TCP_SERVER_H__
#define __RADIXUN_TCP_SERVER_H__

#include "address.h"
#include "socket.h"
#include "../common/iomanager.h"
#include "../common/noncopyable.h"
#include <memory>
#include <functional>

namespace radixun {

//TCP服务器封装
class TcpServer : public std::enable_shared_from_this<TcpServer> , Noncopyable {
public:
    typedef std::shared_ptr<TcpServer> ptr;
    //构造函数
    //socket客户端工作的协程调度器
    //服务器socket执行接收socket连接的协程调度器
    TcpServer(radixun::IOManager* woker = radixun::IOManager::GetThis()
              ,radixun::IOManager* accept_woker = radixun::IOManager::GetThis());
    virtual ~TcpServer();

    virtual bool bind(radixun::Address::ptr addr);
    virtual bool bind(const std::vector<Address::ptr>& addrs
                        ,std::vector<Address::ptr>& fails);
    //启动服务(需要bind成功后执行)
    virtual bool start();
    //停止服务
    virtual void stop();

    
    void setName(const std::string& v) { m_name = v;}
    std::string getName() const { return m_name;}
    void setRecvTimeout(uint64_t v) { m_recvTimeout = v;}
    uint64_t getRecvTimeout() const { return m_recvTimeout;}
    bool isStop() const { return m_isStop;}

protected:
    //处理新连接的Socket类
    virtual void handleClient(Socket::ptr client);
    //开始接受连接
    virtual void startAccept(Socket::ptr sock);
private:
    /// 监听Socket数组
    std::vector<Socket::ptr> m_socks;
    /// 新连接的Socket工作的调度器
    IOManager* m_worker;
    /// 服务器Socket接收连接的调度器
    IOManager* m_acceptWorker;
    /// 接收超时时间(毫秒)
    uint64_t m_recvTimeout;
    /// 服务器名称
    std::string m_name;
    /// 服务是否停止
    bool m_isStop;
};

}

#endif
