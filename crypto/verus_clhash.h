/*
 * This uses veriations of the clhash algorithm for Verus Coin, licensed
 * with the Apache-2.0 open source license.
 * 
 * Copyright (c) 2018 Michael Toutonghi
 * Distributed under the Apache 2.0 software license, available in the original form for clhash
 * here: https://github.com/lemire/clhash/commit/934da700a2a54d8202929a826e2763831bd43cf7#diff-9879d6db96fd29134fc802214163b95a
 * 
 * CLHash is a very fast hashing function that uses the
 * carry-less multiplication and SSE instructions.
 *
 * Original CLHash code (C) 2017, 2018 Daniel Lemire and Owen Kaser
 * Faster 64-bit universal hashing
 * using carry-less multiplications, Journal of Cryptographic Engineering (to appear)
 *
 * Best used on recent x64 processors (Haswell or better).
 *
 **/

#ifndef INCLUDE_VERUS_CLHASH_H
#define INCLUDE_VERUS_CLHASH_H

#include "clhash.h"

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    // Verus Key size must include and be filled with an extra 8 * 128 bytes
    // the excess over power of 2 will not get mutated
    VERUSKEYSIZE=1024 * 8 + (8 * 128)
};

extern thread_local void *verusclhasher_random_data_;
extern thread_local void *verusclhasherrefresh;
extern thread_local int64_t verusclhasher_keySizeInBytes;
extern thread_local uint256 verusclhasher_seed;

uint64_t verusclhash(void * random, const unsigned char buf[64], uint64_t keyMask);

void *alloc_aligned_buffer(uint64_t bufSize);

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus

#include <vector>
#include <string>

// special high speed hasher for VerusHash 2.0
struct verusclhasher {
    int64_t keySizeIn64BitWords;
    int64_t keyMask;

    inline uint64_t keymask(uint64_t keysize)
    {
        int i = 0;
        while (keysize >>= 1)
        {
            i++;
        }
        return i ? (((uint64_t)1) << i) - 1 : 0;
    }

    // align on 128 byte boundary at end
    verusclhasher(uint64_t keysize=VERUSKEYSIZE) : keySizeIn64BitWords((keysize >> 5) << 2)
    {
        // align to 128 bits
        int64_t newKeySize = keySizeIn64BitWords << 3;
        if (verusclhasher_random_data_ && newKeySize != verusclhasher_keySizeInBytes)
        {
            freehashkey();
        }
        // get buffer space for 2 keys
        if (verusclhasher_random_data_ || (verusclhasher_random_data_ = alloc_aligned_buffer(newKeySize << 1)))
        {
            verusclhasherrefresh = ((char *)verusclhasher_random_data_) + newKeySize;
            verusclhasher_keySizeInBytes = newKeySize;
            keyMask = keymask(newKeySize);
        }
    }

    void freehashkey()
    {
        // no chance for double free
        if (verusclhasher_random_data_)
        {
            std::free((void *)verusclhasher_random_data_);
            verusclhasher_random_data_ = NULL;
            verusclhasherrefresh = NULL;
        }
        verusclhasher_keySizeInBytes = 0;
        keySizeIn64BitWords = 0;
        keyMask = 0;
    }

    // this prepares a key for hashing and mutation by copying it from the original key for this block
    // WARNING!! this does not check for NULL ptr, so make sure the buffer is allocated
    inline void *gethashkey()
    {
        memcpy(verusclhasher_random_data_, verusclhasherrefresh, keyMask + 1);
        return verusclhasher_random_data_;
    }

    uint64_t operator()(const unsigned char buf[64]) const {
        return verusclhash(verusclhasher_random_data_, buf, keyMask);
    }
};

#endif // #ifdef __cplusplus

#endif // INCLUDE_VERUS_CLHASH_H
