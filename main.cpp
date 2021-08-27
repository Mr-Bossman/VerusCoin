#include <iostream>
#include "zcash/Address.hpp"
#include "streams.h"
#include "version.h"
#include "utilstrencodings.h"
#include "base58.h"


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
int main(int argc, char *argv[]){

    auto k = libzcash::SproutSpendingKey::random();
    auto addr = k.address();
    std::cout << add_to_string(addr) << std::endl;
    return 0;
}