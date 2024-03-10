#ifndef __RADIXUN_URI_H__
#define __RADIXUN_URI_H__

#include <memory>
#include <string>
#include <stdint.h>
#include "../net/address.h"

namespace radixun {
//URI类
class Uri {
public:
    /// 智能指针类型定义
    typedef std::shared_ptr<Uri> ptr;

    static Uri::ptr Create(const std::string& uri);
    Uri();

    const std::string& getScheme() const { return m_scheme;}
    const std::string& getUserinfo() const { return m_userinfo;}
    const std::string& getHost() const { return m_host;}
    const std::string& getPath() const;
    const std::string& getQuery() const { return m_query;}
    const std::string& getFragment() const { return m_fragment;}
    int32_t getPort() const;
    void setScheme(const std::string& v) { m_scheme = v;}
    void setUserinfo(const std::string& v) { m_userinfo = v;}
    void setHost(const std::string& v) { m_host = v;}
    void setPath(const std::string& v) { m_path = v;}
    void setQuery(const std::string& v) { m_query = v;}
    void setFragment(const std::string& v) { m_fragment = v;}
    void setPort(int32_t v) { m_port = v;}
    //序列化到输出流
    std::ostream& dump(std::ostream& os) const;
    //转成字符串
    std::string toString() const;
    //获取Address
    Address::ptr createAddress() const;
private:
    //是否默认端口
    bool isDefaultPort() const;
private:
    /// schema
    std::string m_scheme;
    /// 用户信息
    std::string m_userinfo;
    /// host
    std::string m_host;
    /// 路径
    std::string m_path;
    /// 查询参数
    std::string m_query;
    /// fragment
    std::string m_fragment;
    /// 端口
    int32_t m_port;
};

}

#endif
