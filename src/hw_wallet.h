


#ifndef HWW_H
#define HWW_H

#include "pubkey.h"
#include "key.h"
namespace HW{
bool has_key(const CPubKey &pub);
size_t index_key(const CPubKey &pub);
inline size_t count_key();
size_t index_key(const CKeyID &pub);
bool has_key(const CKeyID &pub);

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

CKey to_Fpriv(size_t index);
size_t from_Fpriv(const CKey &Fpriv);
int extern_ccSecSig(const unsigned char *msg32, const unsigned char *privateKey, unsigned char **signatureOut);
}

#endif
