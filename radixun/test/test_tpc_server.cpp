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


radixun::Logger::ptr logger = RADIXUN_LOG_ROOT();

void run() {
    auto addr = radixun::Address::LookupAny("0.0.0.0:8033");
    auto addr2 = radixun::UnixAddress::ptr(new radixun::UnixAddress("/tmp/unix_addr"));
    std::vector<radixun::Address::ptr> addrs;
    addrs.push_back(addr);
    addrs.push_back(addr2);

    radixun::TcpServer::ptr tcp_server(new radixun::TcpServer);
    std::vector<radixun::Address::ptr> fails;
    while(!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();
    
}
int main(int argc, char** argv) {
    radixun::IOManager iom(2);
    iom.schedule(run);
    return 0;
}
