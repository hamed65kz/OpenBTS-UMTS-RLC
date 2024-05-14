#ifndef URLCTRANSUM_H
#define URLCTRANSUM_H

#include "../../shareTypes.h"
#include "../URlcBase.h"
#include "../Configs/URlcConfigUm.h"
#include "URlcTransAmUm.h"

class URlcTransUm : // UMTS RLC Unacknowledged Mode Transmitter
    public virtual URlcBase,
    public URlcTransAmUm
{
    URlcConfigUm mConfig;
    // UM Send State Variables
    URlcSN mVTUS;	// SN of next UM PDU to be transmitted.
            // Note: For utran side initial value may not be 0?

    URlcPdu *readLowSidePdu();	// Return a PDU to lower layers, or NULL if queue empty.

    public:
    // We send the RBInfo to URlcConfigUm, but all it uses is the RlcInfo from it.
    URlcTransUm(RBInfo *rbInfo, RrcTfs *dltfs, UEInfo *uep,unsigned dlPduSize, bool isShared=0) :
        URlcBase(URlcModeUm,uep,rbInfo),
        URlcTransAmUm(&mConfig),
        mConfig(*rbInfo,*dltfs,dlPduSize),
        mVTUS(0)
        {mConfig.mIsSharedRlc = isShared;}
    unsigned getRlcHeaderSize() { return 1; }
    unsigned rlcGetDlPduSizeBytes() { return mConfig.mDlPduSizeBytes; }
    void text(std::ostream &os);
};
#endif