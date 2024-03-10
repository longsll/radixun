#ifndef __RADIXUN_HTTP_HTTP_SERVER_H__
#define __RADIXUN_HTTP_HTTP_SERVER_H__

#include "../net/tcp_server.h"
#include "http_session.h"
#include "servlet.h"

namespace radixun {
namespace http {

//HTTP服务器类
//session+servlet
class HttpServer : public TcpServer {
public:
    /// 智能指针类型
    typedef std::shared_ptr<HttpServer> ptr;

    HttpServer(bool keepalive = false
               ,radixun::IOManager* worker = radixun::IOManager::GetThis()
               ,radixun::IOManager* accept_worker = radixun::IOManager::GetThis());

    ServletDispatch::ptr getServletDispatch() const { return m_dispatch;}
    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v;}
protected:
    virtual void handleClient(Socket::ptr client) override;
private:
    /// 是否支持长连接
    bool m_isKeepalive;
    /// Servlet分发器
    ServletDispatch::ptr m_dispatch;
};

}
}

#endif
