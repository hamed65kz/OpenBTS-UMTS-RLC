#ifndef RRCUPLINKMESSAGE_H
#define RRCUPLINKMESSAGE_H

#include "../Utilities/ByteVector.h"


class UEInfo;

// This pops out of the high side of the RLC.
struct RrcUplinkMessage {
    ByteVector *msgSdu;	// memory is managed by the RrcUplinkMessage
    UEInfo *msgUep;
    RbId msgRbid;
    RrcUplinkMessage(ByteVector *wSdu, UEInfo *wUep, RbId wRbid) : msgSdu(wSdu), msgUep(wUep), msgRbid(wRbid) {}
    virtual ~RrcUplinkMessage() { if (msgSdu) delete msgSdu; }
};

#endif