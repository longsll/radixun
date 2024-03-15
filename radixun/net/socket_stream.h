#ifndef __RADIXUN_SOCKET_STREAM_H__
#define __RADIXUN_SOCKET_STREAM_H__

#include "../common/stream.h"
#include "socket.h"

namespace radixun {

//Socket流
class SocketStream : public Stream {
public:
    typedef std::shared_ptr<SocketStream> ptr;

    //owner 是否完全控制
    SocketStream(Socket::ptr sock, bool owner = true);

    //如果m_owner=true,则close
    ~SocketStream();

    virtual int read(void* buffer, size_t length) override;
    virtual int read(ByteArray::ptr ba, size_t length) override;
    virtual int write(const void* buffer, size_t length) override;
    virtual int write(ByteArray::ptr ba, size_t length) override;

    virtual void close() override;
    Socket::ptr getSocket() const { return m_socket;}
    bool isConnected() const;
protected:
    /// Socket类
    Socket::ptr m_socket;
    /// 是否主控
    bool m_owner;
};

}

#endif
