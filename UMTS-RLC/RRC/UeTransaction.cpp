#include "UeTransaction.h"


const char* UEState2Name(UEState state)
{
    switch (state) {
        CASENAME(stUnused)
            CASENAME(stIdleMode)
            CASENAME(stCELL_FACH)
            CASENAME(stCELL_DCH)
            CASENAME(stCELL_PCH)
            CASENAME(stURA_PCH)
            // Do not add a default case here; we want the error message if this
            // function gets out of phase with the enumeration.
    }
    return "unrecognized";
};

UeTransaction::UeTransaction(UEInfo* uep, TransType ttype, RbId wRabMask, unsigned wTransactionId, UEState newState)
{
    //mTransactionId = uep->newTransactionId();
    mTransactionId = wTransactionId;
    mNewState = newState;
    mRabMask = wRabMask;
    mTransactionType = ttype;
    mTransTime.now();
    // Squirrel it away in the UE.
    uep->mTransactions[mTransactionId] = *this;
}

std::string UeTransaction::str()
{
    return format("UeTransaction(type=%d=%s newState=%s rabMask=%d transId=%d)",
        mTransactionType, TransType2Name(mTransactionType),
        UEState2Name(mNewState), mRabMask, mTransactionId);
}

long UeTransaction::elapsed()	// How long since the transaction, in ms?
{
    return mTransTime.elapsed();
}

