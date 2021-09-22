// Copyright (c) 2021 The Dogecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/random/uniform_int.hpp>


#include "policy/policy.h"
#include "arith_uint256.h"
#include "dogecoin.h"
#include "txmempool.h"
#include "util.h"
#include "validation.h"
#include "dogecoin-fees.h"
#include "amount.h"
#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#endif

#ifdef ENABLE_WALLET

CFeeRate GetDogecoinFeeRate(int priority)
{
    switch(priority)
    {
    case SUCH_EXPENSIVE:
        return CFeeRate(COIN / 100 * 521); // 5.21 DOGE, but very carefully avoiding floating point maths
    case MANY_GENEROUS:
        return CFeeRate(CWallet::minTxFee.GetFeePerK() * 100);
    case AMAZE:
        return CFeeRate(CWallet::minTxFee.GetFeePerK() * 10);
    case WOW:
        return CFeeRate(CWallet::minTxFee.GetFeePerK() * 5);
    case MORE:
        return CFeeRate(CWallet::minTxFee.GetFeePerK() * 2);
    case MINIMUM:
    default:
        break;
    }
    return CWallet::minTxFee;
}

const std::string GetDogecoinPriorityLabel(int priority)
{
    switch(priority)
    {
    case SUCH_EXPENSIVE:
        return _("Such expensive");
    case MANY_GENEROUS:
        return _("Many generous");
    case AMAZE:
        return _("Amaze");
    case WOW:
        return _("Wow");
    case MORE:
        return _("More");
    case MINIMUM:
        return _("Minimum");
    default:
        break;
    }
    return _("Default");
}

//mlumin 5/2021: walletfees, all attached to GetDogecoinWalletFeeRate which is just the newly exposed ::minWalletTxFee
CAmount GetDogecoinWalletFee(size_t nBytes_)
{
    //mlumin: super simple fee calc for dogecoin
    CAmount nFee=GetDogecoinWalletFeeRate().GetFee(nBytes_);
    return nFee;
}

//mlumin 5/2021: Establish a wallet rate of n koinu per kb.
//mlumin: this is somewhat redundant to the globally exposed ::minWalletTxFee, but honestly I'd like to have both the rate and amount (with size) here
CFeeRate GetDogecoinWalletFeeRate()
{
    //mlumin 5/2021: currently 1x COIN or 1 dogecoin or 100,000,000 koinu
    return CWallet::minTxFee;
}
#endif

CAmount GetDogecoinMinRelayFee(const CTransaction& tx, unsigned int nBytes, bool fAllowFree)
{
    {
        LOCK(mempool.cs);
        uint256 hash = tx.GetHash();
        double dPriorityDelta = 0;
        CAmount nFeeDelta = 0;
        mempool.ApplyDeltas(hash, dPriorityDelta, nFeeDelta);
        if (dPriorityDelta > 0 || nFeeDelta > 0)
            return 0;
    }

    CAmount nMinFee = ::minRelayTxFeeRate.GetFee(nBytes);
    nMinFee += GetDogecoinDustFee(tx.vout, ::minRelayTxFeeRate);

    if (fAllowFree)
    {
        // There is a free transaction area in blocks created by most miners,
        // * If we are relaying we allow transactions up to DEFAULT_BLOCK_PRIORITY_SIZE - 1000
        //   to be considered to fall into this category. We don't want to encourage sending
        //   multiple transactions instead of one big transaction to avoid fees.
        if (nBytes < (DEFAULT_BLOCK_PRIORITY_SIZE - 1000))
            nMinFee = 0;
    }

    if (!MoneyRange(nMinFee))
        nMinFee = MAX_MONEY;
    return nMinFee;
}

CAmount GetDogecoinDustFee(const std::vector<CTxOut> &vout, CFeeRate &baseFeeRate) {
    CAmount nFee = 0;

    // To limit dust spam, add base fee for each output less than a COIN
    BOOST_FOREACH(const CTxOut& txout, vout)
        if (txout.IsDust(::minRelayTxFeeRate))
            nFee += baseFeeRate.GetFeePerK();

    return nFee;
}