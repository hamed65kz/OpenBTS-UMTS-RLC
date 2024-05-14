#ifndef URLCTRANSAMUM_H
#define URLCTRANSAMUM_H

#include "URlcTrans.h"
#include "../URlcPdu.h"
#include "../Configs/URlcConfigAmUm.h"

class URlcTransAmUm : // Transmit common to AM and UM modes.
        public URlcTrans, public virtual URlcBase
{
protected :
    URlcConfigAmUm *mConfig;

    PduList_t mPduOutQ;

    // For Am and Um modes:
    int mLILeftOver;	// Special case flag carried over from previous PDU.
    // 1 => the previous sdu exactly filled the previous pdu.
    // 2 => the previous sdu was one byte short of filling previous pdu.

    // These vars are required for Am mode but we maintain them for all modes.
    unsigned mVTPDU;	// Used when for Am mode "poll every Poll_PDU" is configured.
    // Incremented for every PDU transmission of any kind.
    // When == Poll_PDU, send a poll and set this to 0.
    // mVTPDU is the absolute PDU count, not modulo arithmetic.


    bool fillPduData(URlcPdu *pdu, unsigned pduHeaderSize,bool*newPdu);		// Fill the pdu with sdu data.



    virtual URlcPdu *readLowSidePdu() = 0;

    unsigned rlcGetBytesAvail();

    // Pull data through the RLC to fill the output queue, up to the specified amt,
    // which is the maximum amount needed for any Transport Format.
    void rlcPullLowSide(unsigned amt);
    unsigned rlcGetPduCnt() { return mPduOutQ.size(); }
    bool pdusFinished();

public:
    void transDoReset();
    // This class is not allocated alone; it is part of URlcTransAm or URlcTransUm.
    URlcTransAmUm(URlcConfigAmUm *wConfig) :
        mConfig(wConfig)
    { transDoReset(); }

    // MAC reads the low side with this.
    URlcBasePdu *rlcReadLowSide();

    // Return the size of the top PDU, or 0 if none.
    unsigned rlcGetFirstPduSizeBits();
    void textAmUm(std::ostream &os);
};
#endif