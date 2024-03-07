#include "log.h"
#include "config.h"
#include "thread.h"
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <assert.h>


radixun::Logger::ptr g_logger = RADIXUN_LOG_ROOT();

int count = 0;
radixun::Mutex s_mutex;

void fun1() {
    RADIXUN_LOG_INFO(g_logger) << "name: " << radixun::Thread::GetName()
                               << "this.name" << radixun::Thread::GetThis()->getName()
                               << "id: " << radixun::GetThreadId()
                               << "this.id: " << radixun::Thread::GetThis()->getId();
    for(int i = 0 ; i < 3 ; i ++){
        radixun::Mutex::Lock lock(s_mutex);
        RADIXUN_LOG_INFO(g_logger) << "+++++++++++++++++++++++++++++";
        ++ count;
    }        
}

void fun2() {

    for(int i = 0 ; i < 1 ; i ++){
        RADIXUN_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void fun3() {
    while(true){
        RADIXUN_LOG_INFO(g_logger) << "=================================";
    }
}

int main()
{
    RADIXUN_LOG_INFO(g_logger) << "thread test begin";
    // YAML::Node root = YAML::LoadFile("./test.yaml");
    // radixun::Config::LoadFromYaml(root);

    std::vector<radixun::Thread::ptr> thrs;
    for(int i = 0 ; i < 3 ; i ++){
        radixun::Thread::ptr thr(new radixun::Thread(&fun1 , "name_" + std::to_string(i)));
        thrs.push_back(thr);
    }
    // std::cout << "Thread::----->" << std::endl;
    for(size_t i = 0 ; i < thrs.size() ;  i++){
        thrs[i]->join();
    }
    RADIXUN_LOG_INFO(g_logger) << "thread test end";
    RADIXUN_LOG_INFO(g_logger) << "count=" << count;

    return 0;
}