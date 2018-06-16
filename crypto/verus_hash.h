// (C) 2018 The Verus Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/*
This provides the PoW hash function for Verus, enabling CPU mining.
*/
#ifndef VERUS_HASH_H_
#define VERUS_HASH_H_

#include <cstring>
#include <vector>

#if !XERCES_HAVE_INTRIN_H
#include <cpuid.h>
#endif

extern "C" 
{
#include "crypto/haraka.h"
#include "crypto/haraka_portable.h"
}

class CVerusHash
{
    public:
        static void Hash(void *result, const void *data, size_t len);

        CVerusHash() {}

        CVerusHash &Write(const unsigned char *data, size_t len);

        CVerusHash &Reset()
        {
            curBuf = buf1;
            result = buf2;
            curPos = 0;
            std::fill(buf1, buf1 + sizeof(buf1), 0);
        }

        void Finalize(unsigned char hash[32])
        {
            if (curPos)
            {
                std::fill(curBuf + 32 + curPos, curBuf + 64, 0);
                haraka512(hash, curBuf);
            }
            else
                std::memcpy(hash, curBuf, 32);
        }

    private:
        // only buf1, the first source, needs to be zero initialized
        unsigned char buf1[64] = {0}, buf2[64];
        unsigned char *curBuf = buf1, *result = buf2;
        size_t curPos = 0;
};

class CVerusHashPortable
{
    public:
        static void Hash(void *result, const void *data, size_t len);

        CVerusHashPortable() {}

        CVerusHashPortable &Write(const unsigned char *data, size_t len);

        CVerusHashPortable &Reset()
        {
            curBuf = buf1;
            result = buf2;
            curPos = 0;
            std::fill(buf1, buf1 + sizeof(buf1), 0);
        }

        void Finalize(unsigned char hash[32])
        {
            if (curPos)
            {
                std::fill(curBuf + 32 + curPos, curBuf + 64, 0);
                haraka512_port(hash, curBuf);
            }
            else
                std::memcpy(hash, curBuf, 32);
        }

    private:
        // only buf1, the first source, needs to be zero initialized
        unsigned char buf1[64] = {0}, buf2[64];
        unsigned char *curBuf = buf1, *result = buf2;
        size_t curPos = 0;
};

extern void verus_hash(void *result, const void *data, size_t len);

inline bool IsCPUVerusOptimized()
{
    unsigned int eax,ebx,ecx,edx;
#ifdef _WIN32
    unsigned int CPUInfo[4];
    CPUInfo[0] = (unsigned int) &eax;
    CPUInfo[1] = (unsigned int) &ebx;
    CPUInfo[2] = (unsigned int) &ecx;
    CPUInfo[3] = (unsigned int) &edx;
    if (!__get_cpuid(CPUInfo, (unsigned int) 1))
#else
    if (!__get_cpuid(1,&eax,&ebx,&ecx,&edx))
#endif
    {
        return false;
    }
    return ((ecx & (bit_AVX | bit_AES)) == (bit_AVX | bit_AES));
};

#endif
