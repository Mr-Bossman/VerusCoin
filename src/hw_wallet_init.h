#include "key_io.h"
#include "wallet/wallet.h"
#include "wallet/walletdb.h"
#ifndef HWW__H
#define HWW__H
namespace HW {
    std::string hw_wallet_connect(CWallet *pwalletMain,std::string port);
}
#endif
