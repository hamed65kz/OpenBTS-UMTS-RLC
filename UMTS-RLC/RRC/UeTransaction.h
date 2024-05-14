#ifndef UET
#define UET
#include "UEDefs.h"

// When we send an RB setup message of some kind, we save the transaction here
// so that we know what to do when we receive the message reply.
// This is necessary for RadioBearerSetup because the UE may request multiple
// setups simultaneously, eg, might ask for two IP addresses at the same time,
// and we need to keep the responses straight.
struct UeTransaction : UEDefs
{
    TransType mTransactionType;	// Event type.
    UEState mNewState;	// If not idle, new state when transaction complete.
    unsigned mRabMask;	// The mask of RBs affected by this transaction.
    uint8_t mTransactionId;
    Timeval mTransTime;	// When the event occurred.

    UeTransaction() : mTransactionType(ttComplete), mNewState(stUnused) {}
    // The rabMask is just extra info saved with the transaction to be used
    // by the transaction response; it is currently a mask of the RABs affected
    // by a radiobearer modification, but conceptually it could be arbitrary info.
    UeTransaction(UEInfo* uep, TransType type, unsigned rabMask, unsigned transactionId,
        UEState nextState = stUnused);
    void transClose() { mTransactionType = ttComplete; }
    long elapsed();	// How long since the transaction, in ms?

    const char* name() { return TransType2Name(mTransactionType); }
    std::string str();
};
#endif
