#include "log.h"
#include "config.h"
#include "thread.h"
#include "fiber.h"
#include "scheduler.h"
#include "iomanager.h"
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <assert.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

radixun::Logger::ptr g_logger = RADIXUN_LOG_ROOT();

int sock = 0;

void test() {
    while(1);
}

void test2(){
    sock = socket(AF_INET, SOCK_STREAM, 0);
    radixun::IOManager::GetThis()->addEvent(sock, radixun::IOManager::WRITE, [](){
        RADIXUN_LOG_INFO(g_logger) << "write callback---------------------------------";
    });  
    radixun::IOManager::GetThis()->addEvent(sock, radixun::IOManager::READ, [](){
        RADIXUN_LOG_INFO(g_logger) << "read callback----------------------------------";
    });    
}

void test_fiber() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "36.155.132.3", &addr.sin_addr.s_addr);
    RADIXUN_LOG_DEBUG(g_logger) << "sock = " << sock;
   
    int rt = !connect(sock, (const sockaddr*)&addr, sizeof(addr));   
      
    if(errno == EINPROGRESS) {
        RADIXUN_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        radixun::IOManager::GetThis()->addEvent(sock, radixun::IOManager::READ, [](){
            RADIXUN_LOG_INFO(g_logger) << "read callback----------------------------------";
        });     
       radixun::IOManager::GetThis()->addEvent(sock, radixun::IOManager::WRITE, [](){
            RADIXUN_LOG_INFO(g_logger) << "write callback---------------------------------";
            radixun::IOManager::GetThis()->cancelEvent(sock, radixun::IOManager::READ);
            close(sock);
        });   
        
    } else{

    }

}

void test_iom(){
    RADIXUN_LOG_DEBUG(g_logger) << "test";
    radixun::IOManager iom(2, true); 
    iom.schedule(&test_fiber);
}

radixun::Timer::ptr s_timer;
void test_timer() {
    radixun::IOManager iom(2 , true , "test_timer");
    s_timer = iom.addTimer(1000, [](){
        static int i = 0;
        RADIXUN_LOG_INFO(g_logger) << "hello timer i=" << i;
        if(++i == 5) {
            // s_timer->reset(2000, true);
            s_timer->cancel();
        }
    }, true);
}

int main()
{
    test_timer();
    // test_iom();
    return 0;
}