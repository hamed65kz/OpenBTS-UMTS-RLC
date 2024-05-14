#ifndef URLCRECVUM_H
#define URLCRECVUM_H

#include "../URlcBase.h"
#include "URlcRecvAmUm.h"
#include "../Configs/URlcConfigUm.h"

class URlcRecvUm : // UMTS RLC Unacknowledged Mode Receiver
    public virtual URlcBase,
    public URlcRecvAmUm
{	public:
    URlcConfigUm mConfig;
    URlcSN mVRUS; 	// SN+1 of last UM PDU received

    URlcRecvUm(RBInfo *rbInfo, RrcTfs *dltfs, UEInfo *uep) :
        URlcBase(URlcModeUm,uep,rbInfo),
        URlcRecvAmUm(&mConfig),
        mConfig(*rbInfo,*dltfs,0), // No downlink pdu size needed in uplink RLC.
        mVRUS(0)
        {}

    void rlcWriteLowSide(const BitVector &pdu);

    //unsigned VRUDR;	// Expected next SN for DAR (duplicate avoidance and reordering.)
    //unsigned VRUDH;	// Highest received SN for DAR
    //unsigned VRUDT;	// Timer for DAR
    //Z100 mTimer_DAR;	// For UM duplicate avoidance and reordering. // see 9.7.10
    //unsigned VRUOH;	// Highest SN received.  Inited per 11.2.3.2
    // Only used for out-of-sequence-delivery
    #if RLC_OUT_OF_SEQ_OPTIONS
    unsigned VRUM() {	// SN of first UM PDU that shall be rejected.
        return addSN(mVRUS,mConfig->mConfigured_Rx_Window_Size);
    }
    #endif
    void text(std::ostream &os);
};
#endif