#include "log.h"
#include "config.h"
#include "thread.h"
#include "fiber.h"
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <assert.h>


radixun::Logger::ptr g_logger = RADIXUN_LOG_ROOT();

void run_in_fiber() {
    RADIXUN_LOG_INFO(g_logger) << "run_in_fiber begin";
    radixun::Fiber::GetThis()->back();
    RADIXUN_LOG_INFO(g_logger) << "run_in_fiber end";
}

void test_fiber() {
    RADIXUN_LOG_INFO(g_logger) << "main begin";

    radixun::Fiber::GetThis();
    radixun::Fiber::ptr fiber(new radixun::Fiber(run_in_fiber));
    fiber->call();
    RADIXUN_LOG_INFO(g_logger) << "fiber call again";
    fiber->call();
    RADIXUN_LOG_INFO(g_logger) << "main end";
}


int main()
{
    radixun::Thread::setName("main");
    radixun::Thread::ptr tr (new radixun::Thread(&test_fiber , "test_fiber"));
    // radixun::Thread::ptr tr2 (new radixun::Thread(&test_fiber , "name__"));
    // radixun::Thread::ptr tr3 (new radixun::Thread(&test_fiber , "name__"));
    tr->join();
    // tr2->join();
    // tr3->join();
    return 0;
}