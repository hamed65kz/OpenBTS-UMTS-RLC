#ifndef URLCPAIR_H
#define URLCPAIR_H
#include "Recv/URlcRecv.h"
#include "Tran/URlcTrans.h"
#include "../RRC/Configs/URRCTrCh.h"

// (pat) Logical channel types.
// 25.331 10.2 Radio Resource Control Messages - each one specifies which logical channel it is sent on.
// Warning: The GSM directory redefines several of these in that namespace.
enum ChannelTypeL3 {
    BCCHType,		// (pat) Used exclusively for SIB messages.  Sent on BCH but may also be sent on FACH (why?)
    CCCHType,		// (pat) For UE in unconnected-mode; Uses SRB0
    MCCHType,
    MSCHType,
    MTCHType,
    CTCHType,
    DTCCH_FACHType,	// (pat) do not need this, use the TrChType.
    DTCCH_RACHType,	// (pat) do not need this, use the TrChType.
    DCCHType,		// (pat) For UE in connected-mode; Uses SRB1, 2, 3, 4
    DTCHType,
    UNDEFINED_CHANNEL
};


class URlcPair
{
public:
    class URlcTrans *mDown;
        class URlcRecv *mUp;
        TrChId mTcid;	// The transport channel to which this RLC is attached.
        URlcPair(RBInfo *rb, RrcTfs *dltfs, UEInfo *uep, TrChId tcid);
        ~URlcPair();
};


unsigned computeDlRlcSize(RBInfo *rb, RrcTfs *dltfs);
const char* URlcMode2Name(URlcMode mode);


extern unsigned macHeaderSize(TrChType trch, ChannelTypeL3 ltype, bool multiplexing);
extern unsigned macHeaderSize(TrChType trch, int rbid, bool multiplexing);

#endif // URLCPAIR_H
