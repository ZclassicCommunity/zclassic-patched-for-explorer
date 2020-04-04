// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_SCRIPT_INTERPRETER_H
#define BITCOIN_SCRIPT_INTERPRETER_H

#include "script_error.h"
#include "script_flags.h"
#include "sighashtype.h"
#include "primitives/transaction.h"

#include <vector>
#include <stdint.h>
#include <string>
#include <climits>

class CPubKey;
class CScript;
class CTransaction;
class uint256;

/** Special case nIn for signing JoinSplits. */
const unsigned int NOT_AN_INPUT = UINT_MAX;

struct PrecomputedTransactionData
{
    uint256 hashPrevouts, hashSequence, hashOutputs, hashJoinSplits, hashShieldedSpends, hashShieldedOutputs;

    PrecomputedTransactionData(const CTransaction& tx);
};

enum SigVersion
{
    SIGVERSION_SPROUT = 0,
    SIGVERSION_OVERWINTER = 1,
    SIGVERSION_SAPLING = 2,
};

uint256 SignatureHash(
    const CScript &scriptCode,
    const CTransaction& txTo,
    unsigned int nIn,
    SigHashType sigHashType,
    const CAmount& amount,
    uint32_t consensusBranchId,
    const PrecomputedTransactionData* cache = NULL);

class BaseSignatureChecker
{
public:
    virtual bool VerifySignature(const std::vector<uint8_t> &vchSig,
                                 const CPubKey &vchPubKey,
                                 const uint256 &sighash) const;

    virtual bool CheckSig(
        const std::vector<unsigned char>& scriptSig,
        const std::vector<unsigned char>& vchPubKey,
        const CScript& scriptCode,
        uint32_t consensusBranchId) const
    {
        return false;
    }

    virtual bool CheckLockTime(const CScriptNum& nLockTime) const
    {
         return false;
    }

    virtual ~BaseSignatureChecker() {}
};

class TransactionSignatureChecker : public BaseSignatureChecker
{
private:
    const CTransaction* txTo;
    unsigned int nIn;
    const CAmount amount;
    const PrecomputedTransactionData* txdata;

public:
    TransactionSignatureChecker(const CTransaction* txToIn, unsigned int nInIn, const CAmount& amountIn) : txTo(txToIn), nIn(nInIn), amount(amountIn), txdata(NULL) {}
    TransactionSignatureChecker(const CTransaction* txToIn, unsigned int nInIn, const CAmount& amountIn, const PrecomputedTransactionData& txdataIn) : txTo(txToIn), nIn(nInIn), amount(amountIn), txdata(&txdataIn) {}

    // The overriden functions are now final.
    bool CheckSig(const std::vector<uint8_t> &scriptSig,
                  const std::vector<uint8_t> &vchPubKey,
                  const CScript &scriptCode,
                  uint32_t consensusBranchId) const final override;
    bool CheckLockTime(const CScriptNum &nLockTime) const final override;
};

class MutableTransactionSignatureChecker : public TransactionSignatureChecker
{
private:
    const CTransaction txTo;

public:
    MutableTransactionSignatureChecker(const CMutableTransaction* txToIn, unsigned int nInIn, const CAmount& amount) : TransactionSignatureChecker(&txTo, nInIn, amount), txTo(*txToIn) {}
};

bool EvalScript(
    std::vector<std::vector<unsigned char> >& stack,
    const CScript& script,
    unsigned int flags,
    const BaseSignatureChecker& checker,
    uint32_t consensusBranchId,
    ScriptError* error = NULL);
bool VerifyScript(
    const CScript& scriptSig,
    const CScript& scriptPubKey,
    unsigned int flags,
    const BaseSignatureChecker& checker,
    uint32_t consensusBranchId,
    ScriptError* serror = NULL);

#endif // BITCOIN_SCRIPT_INTERPRETER_H
