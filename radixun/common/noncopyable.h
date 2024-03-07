#ifndef __RADIXUN_NONCOPYABLE_H__
#define __RADIXUN_NONCOPYABLE_H__

namespace radixun{


/// @brief 无法拷贝对象
class Noncopyable{
public:
    
    Noncopyable() = default;

    ~Noncopyable() = default;

    Noncopyable(const Noncopyable&) = delete;
 
    Noncopyable& operator=(const Noncopyable&) = delete;
};

}

#endif