


#ifndef HWW_H
#define HWW_H
#include <functional>
#include <secp256k1.h>
#include <secp256k1_recovery.h>
namespace HW{
extern secp256k1_context* secp256k1_context_sign;
extern std::function<int(secp256k1_ecdsa_signature *, const unsigned char *, const unsigned char *)> secp256k1_ecdsa_sign;
}
#endif
