#ifndef __RADIXUN_HTTP_SESSION_H__
#define __RADIXUN_HTTP_SESSION_H__

#include "../net/socket_stream.h"
#include "http.h"

namespace radixun {
namespace http {


// HTTPSession封装
// 接收请求报文，发送响应报文
class HttpSession : public SocketStream {
public:
    /// 智能指针类型定义
    typedef std::shared_ptr<HttpSession> ptr;

    //构造函数(owner 是否托管)
    HttpSession(Socket::ptr sock, bool owner = true);

    //接收HTTP请求
    HttpRequest::ptr recvRequest();

    //发送HTTP响应(>0 发送成功 , =0 对方关闭 , <0 Socket异常)
    int sendResponse(HttpResponse::ptr rsp);
};

}
}

#endif
