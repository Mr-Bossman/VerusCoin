/* trick linker */
#include "hw_wallet.h"
namespace HW{
secp256k1_context* secp256k1_context_sign = NULL;
std::function<int(secp256k1_ecdsa_signature *, const unsigned char *, const unsigned char *)> secp256k1_ecdsa_sign;
}