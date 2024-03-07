#ifndef __RADIXUN_TCP_SERVER_H__
#define __RADIXUN_TCP_SERVER_H__

#include <memory>
#include <functional>
#include "address.h"
#include "socket.h"
#include "../common/iomanager.h"
#include "../common/noncopyable.h"

namespace radixun {

/**
 * @brief TCP服务器封装
 */
class TcpServer : public std::enable_shared_from_this<TcpServer> , Noncopyable {
public:
    typedef std::shared_ptr<TcpServer> ptr;
    /**
     * @brief 构造函数
     * @param[in] woker socket客户端工作的协程调度器
     * @param[in] accept_woker 服务器socket执行接收socket连接的协程调度器
     */
    TcpServer(radixun::IOManager* woker = radixun::IOManager::GetThis()
              ,radixun::IOManager* accept_woker = radixun::IOManager::GetThis());

    /**
     * @brief 析构函数
     */
    virtual ~TcpServer();

    /**
     * @brief 绑定地址
     * @return 返回是否绑定成功
     */
    virtual bool bind(radixun::Address::ptr addr);

    /**
     * @brief 绑定地址数组
     * @param[in] addrs 需要绑定的地址数组
     * @param[out] fails 绑定失败的地址
     * @return 是否绑定成功
     */
    virtual bool bind(const std::vector<Address::ptr>& addrs
                        ,std::vector<Address::ptr>& fails);

    /**
     * @brief 启动服务
     * @pre 需要bind成功后执行
     */
    virtual bool start();

    /**
     * @brief 停止服务
     */
    virtual void stop();

    /**
     * @brief 返回读取超时时间(毫秒)
     */
    uint64_t getRecvTimeout() const { return m_recvTimeout;}

    /**
     * @brief 返回服务器名称
     */
    std::string getName() const { return m_name;}

    /**
     * @brief 设置读取超时时间(毫秒)
     */
    void setRecvTimeout(uint64_t v) { m_recvTimeout = v;}

    /**
     * @brief 设置服务器名称
     */
    void setName(const std::string& v) { m_name = v;}

    /**
     * @brief 是否停止
     */
    bool isStop() const { return m_isStop;}
protected:

    /**
     * @brief 处理新连接的Socket类
     */
    virtual void handleClient(Socket::ptr client);

    /**
     * @brief 开始接受连接
     */
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
