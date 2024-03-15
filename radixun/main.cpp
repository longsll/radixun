#include "radixun.h"


radixun::Logger::ptr logger = RADIXUN_LOG_ROOT();

void run() {
    auto addr = radixun::Address::LookupAny("0.0.0.0:8033");
    radixun::TcpServer::ptr tcp_server(new radixun::TcpServer);
    while(!tcp_server->bind(addr)) {
        sleep(2);
    }
    tcp_server->start();
    
}
int main(int argc, char** argv) {
    radixun::IOManager iom(2);
    iom.schedule(run);
    return 0;
}
