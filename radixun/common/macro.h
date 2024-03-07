#ifndef __RADIXUN_MACRO_H__
#define __RADIXUN_MACRO_H__

#include <string.h>
#include <assert.h>
#include "util.h"

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#   define RADIXUN_LICKLY(x)       __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#   define RADIXUN_UNLICKLY(x)     __builtin_expect(!!(x), 0)
#else
#   define RADIXUN_LICKLY(x)      (x)
#   define RADIXUN_UNLICKLY(x)      (x)
#endif


/// 断言宏封装
#define RADIXUN_ASSERT(x) \
    if(RADIXUN_UNLICKLY(!(x))) { \
        RADIXUN_LOG_ERROR(RADIXUN_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << radixun::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

/// 断言宏封装
#define RADIXUN_ASSERT2(x, w) \
    if(RADIXUN_UNLICKLY(!(x))) { \
        RADIXUN_LOG_ERROR(RADIXUN_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << radixun::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#endif