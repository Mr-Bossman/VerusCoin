#include <iostream>
#include "zcash/Address.hpp"
#include "streams.h"
#include "version.h"
#include "utilstrencodings.h"
#include "base58.h"
#include "crypto/sha256.h"
__attribute__((weak))  void OPENSSL_cleanse(void *ptr,size_t len){
    memset(ptr,0,len);
}

std::string add_to_string(libzcash::SproutPaymentAddress addr){
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << addr;
    std::vector<unsigned char> data;
    data.insert(data.end(),ss.begin(),ss.end());
    std::vector<char> s;
    s.resize(128);
    size_t i =128;
    if(!b58enc(reinterpret_cast<char*>(s.data()),&i,reinterpret_cast<unsigned char*>(data.data()),data.size()))
    return "fail";
    s.resize(i);
    return std::string(s.begin(),s.end());
}

uint252 uint252_sha256(std::string str)
{
    uint256 hash;
    CSHA256().Write((unsigned char*)(str.c_str()), str.length()).Finalize(hash.begin());
    (*hash.begin()) &= 0x0F;
    return uint252(hash);
}

int bust(){

    std::string str = "crust";
    auto k = libzcash::SproutSpendingKey(uint252_sha256(str));
    auto addr = k.address();
    std::cout << add_to_string(addr) << std::endl;
    return 0;
}
extern "C"{
    int crust(){
        return bust();
    }
}