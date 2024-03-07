#ifndef __RADIXUN_UTIL_H__
#define __RADIXUN_UTIL_H__

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>
#include "config.h"

namespace radixun{

    //返回当前线程的ID
    pid_t GetThreadId();

    //返回当前协程的ID
    uint32_t GetFiberId();

    /**
     * @brief 获取当前的调用栈
     * @param[out] bt 保存调用栈
     * @param[in] size 最多返回层数
     * @param[in] skip 跳过栈顶的层数
     */
    void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);

    /**
     * @brief 获取当前栈信息的字符串
     * @param[in] size 栈的最大层数
     * @param[in] skip 跳过栈顶的层数
     * @param[in] prefix 栈信息前输出的内容
     */
    std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

    /**
     * @brief 获取当前时间的毫秒
     */
    uint64_t GetCurrentMS();

    /**
     * @brief 获取当前时间的微秒
     */
    uint64_t GetCurrentUS();

    template<class V, class Map, class K>
    V GetParamValue(const Map& m, const K& k, const V& def = V()) {
        auto it = m.find(k);
        if(it == m.end()) {
            return def;
        }
        try {
            return boost::lexical_cast<V>(it->second);
        } catch (...) {
        }
        return def;
    }

    template<class V, class Map, class K>
    bool CheckGetParamValue(const Map& m, const K& k, V& v) {
        auto it = m.find(k);
        if(it == m.end()) {
            return false;
        }
        try {
            v = boost::lexical_cast<V>(it->second);
            return true;
        } catch (...) {
        }
        return false;
}

}


#endif
