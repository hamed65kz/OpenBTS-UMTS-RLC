#include "URlcAm.h"
#include "../Utilities/Logger.h"

URlcAm::URlcAm(RBInfo *rbInfo,RrcTfs *dltfs,UEInfo *uep,unsigned dlPduSize) :
        URlcBase(URlcModeAm,uep,rbInfo),
        URlcTransAm(&mConfig),// Warning, this is not set up yet, but gcc whines if you order correctly.
        URlcRecvAm(&mConfig),
        mConfig(*rbInfo,*dltfs,dlPduSize)
    {
        mAmid=format("AM%d",mrbid);
        transAmInit();
        recvAmInit();
        if (mConfig.mMaxRST == 0) {
            LOG(WARNING) << "Max_RESET not configured";
        }
    }

void URlcAm::recvResetPdu(URlcPdu *pdu)
{
    unsigned rsn = pdu->getBit(4);
    unsigned hfn = pdu->getField(8,20);
    RLCLOG("receive reset pdu cnt=%d rsn=%d ResetAckRSN=%d",mResetRecvCount,rsn,mResetAckRSN);
    // If resetInProgress, we already sent a reset, and now we have received one back,
    // which corresponds to clause 11.4.5.3. Do the reset now.
    if (resetInProgress() || mResetRecvCount == 0 || rsn != mResetAckRSN) {
        // This is a new reset.
        RLCLOG("full reset");
        transAmReset();
        recvAmReset();
    } else {
        // Redundant reset message received.
        // Just resend another reset ack and we are done.
    }
    mULHFN = hfn;
    mResetAckRSN = rsn;	// RSN in outgoing reset ack must match most recent incoming reset.
    mResetRecvCount++;	// Counts all resets received ever, unlike mVTRST counts resents of this reset
    mSendResetAck = true;
}

// 11.4.4
void URlcAm::recvResetAck(URlcPdu *pdu)
{
    unsigned rsn = pdu->getBit(4);
    RLCLOG("receive reset_ack pdu rsn=%d ResetTransRSN=%d",rsn,mResetTransRSN);
    // unused unsigned hfn = pdu->getField(8,20);
    if (! resetInProgress()) {
        // This is nothing to worry about because they can pass each other in flight.
        LOG(INFO) <<"Discarding Reset_Ack pdu that does not correspond to a Reset";
        return;
    }
    if (rsn != mResetTransRSN) {
        // This is slightly disturbing, but we'll press on.
        LOG(INFO) <<"Discarding Reset_Ack pdu with invalid rsn";
        return;
    }
    transAmReset();
    recvAmReset();
    mVTRST = 0;
    mTimer_RST.reset();
    mResetTransRSN++;
    mULHFN++;
    mDLHFN++;
}


