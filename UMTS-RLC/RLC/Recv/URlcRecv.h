// Any mode receiver.
#ifndef URLCRECV_H
#define URLCRECV_H

#include "../URlcBase.h"
#include "string.h"
#include "../../Utilities/Interthread.h"
#include "../RrcUplinkMessage.h"

using namespace  std;

// The Uplink SDU is just a ByteVector.
typedef ByteVector URlcUpSdu;

static InterthreadQueue<RrcUplinkMessage> gRrcUplinkQueue;
static InterthreadQueue<RrcUplinkMessage> gSgsnUplinkQueue;

class URlcRecv :
    public virtual URlcBase
{
protected:


    URlcHighSideFuncType mHighSideFunc;
    string mRlcid;

    // This is where outgoing SDUs arrive.
    // The SDU is allocated and must be deleted eventually.
    void rlcSendHighSide(URlcUpSdu *sdu);

    public:
    // This is where pdus come in from the MAC via a routine in the UEInfo
    // to map to the approriate RLC entity based on the RbId.
    virtual void rlcWriteLowSide(const BitVector &pdu) = 0;

    // This is used for testing.
    void rlcSetHighSide(URlcHighSideFuncType wHighSideFunc) { mHighSideFunc = wHighSideFunc; }

    URlcRecv() : mHighSideFunc(0) {}
    const char *rlcid() { return mRlcid.c_str(); }
    virtual void text(std::ostream &os) = 0;
};
#endif