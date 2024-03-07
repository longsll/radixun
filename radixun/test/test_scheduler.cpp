#include "log.h"
#include "config.h"
#include "thread.h"
#include "fiber.h"
#include "scheduler.h"
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <assert.h>

radixun::Logger::ptr g_logger = RADIXUN_LOG_ROOT();

void test_fiber() {
    static int s_count = 1;
    RADIXUN_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    sleep(1);
    if(--s_count >= 0) {
        radixun::Scheduler::GetThis()->schedule(&test_fiber, radixun::GetThreadId());
    }
}


void test_fiber2() {
    static int s_count = 1;
    RADIXUN_LOG_INFO(g_logger) << "test in fiber2 s_count=" << s_count;

    sleep(1);
    if(--s_count >= 0) {
        radixun::Scheduler::GetThis()->schedule(&test_fiber, radixun::GetThreadId());
    }
}

int main()
{
    RADIXUN_LOG_INFO(g_logger) << "main star";
    radixun::Scheduler sc(2 , false , "test");
    sc.start();
    sleep(2);
    RADIXUN_LOG_INFO(g_logger) << "scheduler";
    sc.schedule(&test_fiber);
    sc.schedule(&test_fiber2);
    sc.stop();
    RADIXUN_LOG_INFO(g_logger) << "over";
    return 0;
}