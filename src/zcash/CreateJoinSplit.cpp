// Copyright (c) 2016 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "../util.h"
#include "primitives/transaction.h"
#include "zcash/JoinSplit.hpp"

#include "libsnark/common/profiling.hpp"
#include "komodo_defs.h"
char ASSETCHAINS_SYMBOL[KOMODO_ASSETCHAIN_MAXLEN];
int64_t MAX_MONEY = 200000000 * 100000000LL;
uint16_t BITCOIND_PORT = 7771;

using namespace libzcash;

int main(int argc, char **argv)
{
    libsnark::start_profiling();

    auto p = ZCJoinSplit::Prepared((ZC_GetParamsDir() / "sprout-verifying.key").string(),
                                   (ZC_GetParamsDir() / "sprout-proving.key").string());

    // construct a proof.

    for (int i = 0; i < 5; i++) {
        uint256 anchor = ZCIncrementalMerkleTree().root();
        uint256 pubKeyHash;

        JSDescription jsdesc(*p,
                             pubKeyHash,
                             anchor,
                             {JSInput(), JSInput()},
                             {JSOutput(), JSOutput()},
                             0,
                             0);
    }

    delete p; // not that it matters
}