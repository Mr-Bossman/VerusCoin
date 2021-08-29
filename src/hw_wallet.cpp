#include "hw_wallet.h"
#include "hw_wallet_init.h"

#include "pubkey.h"
#include <iostream>
#include <vector>
#include <type_traits>
#include <secp256k1.h>
#include <secp256k1_recovery.h>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include "ui_interface.h"
#include "serial/serial.h"
#include "keystore.h"

namespace HW{

/* static*/
static std::vector<std::vector<unsigned char>> keys;
static serial::Serial port;


/* static decl*/

static void ECC_Start();


template <class T>
std::vector<T> get_pubs();

template <class T>
T get_index(size_t index);

template <>
CKeyID get_index<CKeyID>(size_t index);

template <>
CPubKey get_index<CPubKey>(size_t index);

template <>
std::vector<CKeyID> get_pubs<CKeyID>();

template <>
std::vector<CPubKey> get_pubs<CPubKey>();

static bool has_key(const CPubKey &pub);
static bool has_key(const CKeyID &pub);
static size_t index_key(const CPubKey &pub);
static size_t index_key(const CKeyID &pub);
static inline size_t count_key();
static size_t from_Fpriv(const CKey &Fpriv);
static CKey to_Fpriv(size_t index);

static void write(std::string str);
static std::string set_port(std::string str);
static std::vector<std::vector<unsigned char>> req_pubs();
int secp256k1_ecdsa_sign_int(secp256k1_ecdsa_signature *signature, const unsigned char *msg32, const unsigned char *seckey);
std::string hw_wallet_connect(CWallet *pwalletMain,std::string port);



/* defs */
static bool has_key(const CPubKey &pub){
    int b = -1;
    size_t i = 0;
    do{
        if(i>=keys.size())break;
    }while(b = memcmp(pub.begin(),reinterpret_cast<unsigned char*>(keys[i++].data()),33));
    return b == 0;
}

static bool has_key(const CKeyID &pub){
    size_t i = 0;
    bool b = false;
    while(!b && i <count_key()){
        CKeyID tmp = CKeyID(Hash160(keys[i]));
        b = (tmp == pub);
        i++;
    }
    return b;
}

static size_t index_key(const CPubKey &pub){
    size_t i = 0;
    do{
        if(i>=keys.size())break;
    }while(memcmp(pub.begin(),reinterpret_cast<unsigned char*>(keys[i++].data()),33));
    return i;
}

static size_t index_key(const CKeyID &pub){
    size_t i = 0;
    bool b = false;
    while(!b && i <count_key()){
        CKeyID tmp = CKeyID(Hash160(keys[i]));
        b = (tmp == pub);
        i++;
    }
    return i;
}

static inline size_t count_key(){
    return keys.size();
}


template <>
CKeyID get_index<CKeyID>(size_t index){
    if(index < count_key())
        return CKeyID(Hash160(keys[index]));
    else
    return CKeyID();
}

template <>
CPubKey get_index<CPubKey>(size_t index){
    if(index < count_key())
        return CPubKey(keys[index]);
    else return CPubKey();
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

static CKey to_Fpriv(size_t index){
    CKey newc;
    unsigned char val[32] = {0};
    *((size_t*)val) = index;
    newc.SetUnsafe(&val[0],&val[32],false);
    newc.hidden_pub = get_index<CPubKey>(index);
    newc.external_ = true;
    return newc;
}

static size_t from_Fpriv(const CKey &Fpriv){
    return *((size_t*)Fpriv.begin());
}

static uint8_t hex_byte(uint8_t *str)
{
  char tmp[3] = {0};
  tmp[0] = str[0];
  tmp[1] = str[1];
  unsigned long int ret;
  ret = strtoul(tmp, NULL, 16);
  return (uint8_t)ret;
}

static void ECC_Start() {
    assert(secp256k1_context_sign == NULL);

    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    assert(ctx != NULL);

    {
        // Pass in a random blinding seed to the secp256k1 context.
        unsigned char seed[32];
        LockObject(seed);
        GetRandBytes(seed, 32);
        bool ret = secp256k1_context_randomize(ctx, seed);
        assert(ret);
        UnlockObject(seed);
    }

    secp256k1_context_sign = ctx;
}

static void write(std::string str){
    if(port.isOpen())
    for(auto c: str){
        port.write(std::string(1, c));
        boost::this_thread::sleep_for(boost::chrono::milliseconds(75));
    }
}

static std::string set_port(std::string str){
    try {
        if(port.getPort() != "")
            port.close();
        port.setPort(str);
        port.setBaudrate(115200);
        port.open();
    } catch (exception &e) {
        return e.what();
    }
    return "";
}

static std::vector<std::vector<unsigned char>> req_pubs(){
    write("pub\n\r\r");
    if(port.isOpen()){
    std::vector<std::string> raw = port.readlines();
        for(auto line : raw){
            if(line.length() == 72){
                keys.push_back(ParseHex(line.substr(4,72)));
            }
        }
    }
    return keys;
}

static void sel(const size_t index){
    std::string sels = "sel " + std::to_string(index+1) + "\n\r\r";
    write(sels);
}

int secp256k1_ecdsa_sign_int(secp256k1_ecdsa_signature *signature, const unsigned char *msg32, const unsigned char *seckey){
    size_t index = *((size_t*)seckey);
    sel(index);
    std::string sig = "signh bruh " + HexStr<const unsigned char*>(msg32,msg32+32) + "\n\r\r";
    write(sig);
    if(port.isOpen()){
        std::vector<std::string> raw = port.readlines();
        for(auto line : raw){
            if(line.length() == 130) {
                auto tmp = ParseHex(line);
                for(size_t i = 0; i < 64; i++){
                    ((unsigned char*)signature)[i] = tmp[i];
                }
            }
        }
    }else {
        return 0;
    }
    return 1;
}

int secp256k1_ecdsa_sign_recoverable_int(secp256k1_ecdsa_recoverable_signature *signature, const unsigned char *msg32, const unsigned char *seckey){
    size_t index = *((size_t*)seckey);
    sel(index);
    std::string sig = "signh bruh " + HexStr<const unsigned char*>(msg32,msg32+32) + "\n\r\r";
    write(sig);
    if(port.isOpen()){
        std::vector<std::string> raw = port.readlines();
        for(auto line : raw){
            if(line.length() == 130) {
                auto tmp = ParseHex(line);
                for(size_t i = 0; i < 64; i++){
                    ((unsigned char*)signature)[i] = tmp[i];
                }
            }
        }
    }else {
        return 0;
    }
    return 1;
}
std::string hw_wallet_connect(CWallet *pwalletMain,std::string port){
    LOCK2(cs_main, pwalletMain->cs_wallet);
    HW::secp256k1_ecdsa_sign = secp256k1_ecdsa_sign_int;
    HW::secp256k1_ecdsa_sign_recoverable = secp256k1_ecdsa_sign_recoverable_int;
    std::string ret = "";
    if(secp256k1_context_sign != NULL)
        ECC_Start();
    ret += set_port(port);
    if(ret.length())
        return ret;
    try{
    req_pubs();
    } catch (exception &e) {
        return e.what();
    }

    for(size_t i = 0; i < count_key(); i++){
        {
        CKeyID cur = get_index<CKeyID>(i);
        CScript script;
        if (IsValidDestination(cur))
            script = GetScriptForDestination(cur);
        else
            return "internal script err";

        if (::IsMine(*pwalletMain, script) == ISMINE_SPENDABLE)
            continue;
                    pwalletMain->MarkDirty();
        {
            pwalletMain->LoadKey(to_Fpriv(i),get_index<CPubKey>(i));
            bool fUpdated = false;
            {
                LOCK(pwalletMain->cs_wallet); // mapAddressBook
                std::map<CTxDestination, CAddressBookData>::iterator mi = pwalletMain->mapAddressBook.find(cur);
                fUpdated = mi != pwalletMain->mapAddressBook.end();
                pwalletMain->mapAddressBook[cur].name = "";
            }
            pwalletMain->NotifyAddressBookChanged(pwalletMain,cur, "", true, "receive",(fUpdated)? CT_UPDATED : CT_NEW);
        }
        pwalletMain->mapKeyMetadata[cur].nCreateTime = 1;
        pwalletMain->nTimeFirstKey = 1;
        pwalletMain->ScanForWalletTransactions(chainActive.Genesis(), true);
        pwalletMain->ReacceptWalletTransactions();
        }
    }
    return "sucess";
}
}