#include "radixun.h"

radixun::Logger::ptr logger = RADIXUN_LOG_ROOT();

void test() {
    std::vector<radixun::Address::ptr> addrs;
    radixun::Address::ptr addr;
    
    //通过host地址返回对应条件的所有Address
    bool v = radixun::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    if(!v) {
        RADIXUN_LOG_ERROR(logger) << "lookup fail";
        return;
    }
    RADIXUN_LOG_INFO(logger) << "Look up all";
    for(size_t i = 0; i < addrs.size(); ++i) {
        RADIXUN_LOG_INFO(logger) << i << " - " << addrs[i]->toString();
    }
    //any address
    addr = radixun::Address::LookupAny("www.bilibili.com");
    RADIXUN_LOG_INFO(logger) << "Look up any";
    RADIXUN_LOG_INFO(logger) << addr->toString();
    // this ip
    RADIXUN_LOG_INFO(logger) << "this ip";
    std::multimap<std::string,std::pair<radixun::Address::ptr, uint32_t> >mp;
    radixun::Address::GetInterfaceAddresses(mp);
    for(auto [x , y] : mp){
        RADIXUN_LOG_INFO(logger) << x << " " << y.first->toString() << " " 
        << y.second;
    }
    //ens33
    RADIXUN_LOG_INFO(logger) << "this ip *";
    std::vector<std::pair<radixun::Address::ptr, uint32_t> > vec;
    radixun::Address::GetInterfaceAddresses(vec , "*");   
    for(auto x : vec){
        RADIXUN_LOG_INFO(logger) << x.first->toString() << " " << x.second;
    } 
}

void test_iface() {
    std::multimap<std::string, std::pair<radixun::Address::ptr, uint32_t> > results;
    //获取指定网卡的地址和子网掩码位数
    bool v = radixun::Address::GetInterfaceAddresses(results);
    if(!v) {
        RADIXUN_LOG_ERROR(logger) << "GetInterfaceAddresses fail";
        return;
    }
    for(auto& i: results) {
        RADIXUN_LOG_INFO(logger) << i.first << " - " << i.second.first->toString() << " - "
            << i.second.second;
    }
}

void test_ipv4() {
    // auto addr = radixun::IPAddress::Create("www.baidu.com" , 80);
    auto addr = radixun::IPAddress::Create("112.80.248.75", 80);
    auto saddr = addr->subnetMask(24);
    auto baddr = addr->broadcastAddress(24);
    auto naddr = addr->networdAddress(24);
    if(addr) {
        RADIXUN_LOG_INFO(logger) << addr->toString();
        RADIXUN_LOG_INFO(logger) << saddr->toString();
        RADIXUN_LOG_INFO(logger) << baddr->toString();
        RADIXUN_LOG_INFO(logger) << naddr->toString();
    }
}

void test_ipv6(){
    auto addr = radixun::IPAddress::Create("fe80::215:5dff:fe20:e26a", 8020);
    auto saddr = addr->subnetMask(64);
    auto baddr = addr->broadcastAddress(64);
    auto naddr = addr->networdAddress(64);
        if(addr) {
        RADIXUN_LOG_INFO(logger) << addr->toString();
        RADIXUN_LOG_INFO(logger) << saddr->toString();
        RADIXUN_LOG_INFO(logger) << baddr->toString();
        RADIXUN_LOG_INFO(logger) << naddr->toString();
    }
}

int main(int argc, char** argv) {
    test_ipv4();
    test_ipv6();
    // test_iface();
    // test();
    return 0;
}
