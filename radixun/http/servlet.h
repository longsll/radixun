#ifndef __RADIXUN_HTTP_SERVLET_H__
#define __RADIXUN_HTTP_SERVLET_H__

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include "http.h"
#include "http_session.h"
#include "../common/thread.h"

namespace radixun {
namespace http {

//Servlet封装
class Servlet {
public:
    /// 智能指针类型定义
    typedef std::shared_ptr<Servlet> ptr;

    Servlet(const std::string& name)
        :m_name(name) {}
    virtual ~Servlet() {}

    //处理请求
    virtual int32_t handle(radixun::http::HttpRequest::ptr request
                   , radixun::http::HttpResponse::ptr response
                   , radixun::http::HttpSession::ptr session) = 0;

    const std::string& getName() const { return m_name;}
protected:
    /// 名称
    std::string m_name;
};

//函数式Servlet
class FunctionServlet : public Servlet {
public:
    /// 智能指针类型定义
    typedef std::shared_ptr<FunctionServlet> ptr;
    /// 函数回调类型定义
    typedef std::function<int32_t (radixun::http::HttpRequest::ptr request
                   , radixun::http::HttpResponse::ptr response
                   , radixun::http::HttpSession::ptr session)> callback;

    FunctionServlet(callback cb);
    virtual int32_t handle(radixun::http::HttpRequest::ptr request
                   , radixun::http::HttpResponse::ptr response
                   , radixun::http::HttpSession::ptr session) override;
private:
    /// 回调函数
    callback m_cb;
};

//Servlet分发器
class ServletDispatch : public Servlet {
public:
    /// 智能指针类型定义
    typedef std::shared_ptr<ServletDispatch> ptr;
    /// 读写锁类型定义
    typedef RWMutex RWMutexType;

    //构造函数
    ServletDispatch();
    virtual int32_t handle(radixun::http::HttpRequest::ptr request
                   , radixun::http::HttpResponse::ptr response
                   , radixun::http::HttpSession::ptr session) override;

    //添加servlet
    void addServlet(const std::string& uri, Servlet::ptr slt);
    //添加servlet
    void addServlet(const std::string& uri, FunctionServlet::callback cb);
    //添加模糊匹配servlet
    void addGlobServlet(const std::string& uri, Servlet::ptr slt);
    //添加模糊匹配servlet
    void addGlobServlet(const std::string& uri, FunctionServlet::callback cb);
    //删除servlet
    void delServlet(const std::string& uri);
    //删除模糊匹配servlet
    void delGlobServlet(const std::string& uri);
    //返回默认servlet
    Servlet::ptr getDefault() const { return m_default;}
    //设置默认servlet
    void setDefault(Servlet::ptr v) { m_default = v;}
    //通过uri获取servlet
    Servlet::ptr getServlet(const std::string& uri);
    //通过uri获取模糊匹配servlet
    Servlet::ptr getGlobServlet(const std::string& uri);
    //通过uri获取servlet
    Servlet::ptr getMatchedServlet(const std::string& uri);
private:
    /// 读写互斥量
    RWMutexType m_mutex;
    /// 精准匹配servlet MAP
    /// uri(/radixun/xxx) -> servlet
    std::unordered_map<std::string, Servlet::ptr> m_datas;
    /// 模糊匹配servlet 数组
    /// uri(/radixun/*) -> servlet
    std::vector<std::pair<std::string, Servlet::ptr> > m_globs;
    /// 默认servlet，所有路径都没匹配到时使用
    Servlet::ptr m_default;
};

//NotFoundServlet(默认返回404)
class NotFoundServlet : public Servlet {
public:
    /// 智能指针类型定义
    typedef std::shared_ptr<NotFoundServlet> ptr;
    NotFoundServlet();
    virtual int32_t handle(radixun::http::HttpRequest::ptr request
                   , radixun::http::HttpResponse::ptr response
                   , radixun::http::HttpSession::ptr session) override;

};

}
}

#endif
