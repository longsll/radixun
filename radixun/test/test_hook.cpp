#include "radixun.h"
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


radixun::Logger::ptr g_logge = RADIXUN_LOG_ROOT();

void test_sleep() {
    radixun::IOManager iom(1);
    iom.schedule([](){
        sleep(2);
        RADIXUN_LOG_INFO(g_logge) << "sleep 2";
    });

    iom.schedule([](){
        sleep(3);
        RADIXUN_LOG_INFO(g_logge) << "sleep 3";
    });
    RADIXUN_LOG_INFO(g_logge) << "test_sleep";
}

void test_sock() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "36.155.132.3", &addr.sin_addr.s_addr);

    RADIXUN_LOG_INFO(g_logge) << "begin connect";
    int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    RADIXUN_LOG_INFO(g_logge) << "connect rt=" << rt << " errno=" << errno;

    if(rt) {
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    RADIXUN_LOG_INFO(g_logge) << "send rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    RADIXUN_LOG_INFO(g_logge) << "recv rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    buff.resize(rt);
    RADIXUN_LOG_INFO(g_logge) << buff;
}

int main(int argc, char** argv) {
    //test_sleep();
    radixun::IOManager iom;
    iom.schedule(test_sock);
    return 0;
}
