#include "hw_wallet.h"
#include "hw_wallet_init.h"

#include "pubkey.h"
#include <iostream>
#include <vector>
#include <type_traits>

namespace HW{
unsigned char key[1][33];
unsigned char dont_mind_me[1][32];

bool has_key(const CPubKey &pub){
    int b = -1;
    size_t i = 0;
    while(b = memcmp(pub.begin(),key[i++],33));
    return b == 0;
}
size_t index_key(const CPubKey &pub){
    size_t i = 0;
    while(memcmp(pub.begin(),key[i++],33));
    return i;
}

inline size_t count_key(){
    return (sizeof(key)/33);
}
size_t index_key(const CKeyID &pub){
    size_t i = 0;
    bool b = false;
    while(!b && i <count_key()){
        CKeyID tmp = CKeyID(Hash160(key[i], key[i] + 33));
        b = (tmp == pub);
        i++;
    }
    return i;
}

bool has_key(const CKeyID &pub){
    size_t i = 0;
    bool b = false;
    while(!b && i <count_key()){
        CKeyID tmp = CKeyID(Hash160(key[i], key[i] + 33));
        b = (tmp == pub);
        i++;
    }
    return b;
}

template <>
CKeyID get_index<CKeyID>(size_t index){
        return CKeyID(Hash160(key[index], key[index] + 33));
}

template <>
CPubKey get_index<CPubKey>(size_t index){
        return CPubKey(key[index], key[index] + 33);
}


template <>
std::vector<CKeyID> get_pubs<CKeyID>(){
    std::vector<CKeyID> ret;
    for(size_t ind = 0; ind < count_key();ind++)
        ret.push_back(get_index<CKeyID>(ind));
    return ret;
}

template <>
std::vector<CPubKey> get_pubs<CPubKey>(){
    std::vector<CPubKey> ret;
    for(size_t ind = 0; ind < count_key();ind++)
        ret.push_back(get_index<CPubKey>(ind));
    return ret;
}

CKey to_Fpriv(size_t index){
    CKey newc;
    unsigned char val[32] = {0};
    *((size_t*)val) = index;
    newc.Set(&val[0],&val[32],true);
    newc.external_ = true;
    return newc;
}

size_t from_Fpriv(const CKey &Fpriv){
    return *((size_t*)Fpriv.begin());
}

int extern_ccSecSig(const unsigned char *msg32, const unsigned char *privateKey, unsigned char **signatureOut){
    return cc_MakeSecp256k1Signature(msg32,dont_mind_me[*((size_t*)privateKey)],signatureOut);
}
void hw_wallet_init(CWallet *pwalletMain){
    memcpy(key[0],reinterpret_cast<char*>(ParseHex("0317bf642dbbe2ed0fb804d81112218721430ae5fbaec1eda3fc9a1f660b7034da").data()),33);
    memcpy(dont_mind_me[0],reinterpret_cast<char*>(ParseHex("408f31d86c6bf4a8aff4ea682ad002278f8cb39dc5f37b53d343e63a61f3cc4f").data()),32);
    for (int i = 0; i < 33; i++)
    printf("%02x", (int)key[0][i]);
    std::cout << std::endl;
    for(size_t i = 0; i < count_key(); i++){
        std::cout << pwalletMain->LoadKey(to_Fpriv(i),get_index<CPubKey>(i))<< std::endl;
    }
}
}