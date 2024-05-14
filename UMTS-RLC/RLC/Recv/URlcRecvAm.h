#ifndef URLCRECVAM_H
#define URLCRECVAM_H


#include "URlcRecvAmUm.h"
//#include "../Tran/URlcTransAmUm.h"
//#include "../Tran/URlcTransAm.h"
#include "../Configs/URlcConfigAm.h"
//#include "../URlcAm.h"

class URlcAm;
class URlcTransAm;
class URlcRecvAmUm;

class URlcRecvAm : // UMTS RLC Acknowledged Mode Receiver
    public URlcRecvAmUm
{
    friend class URlcTransAm;
    friend class URlcRecvAmUm;
protected:
    URlcConfigAm *mConfig;
    // This class is not allocated alone; it is part of URlcAm.

    //URlcTransAm *mTrans;	// Pointer to the paired transmitting entity.

    // GSM25.322 9.4: AM Receive State Variables.
    // The range from mVRR to mVRH is what we need to acknowledge to the peer.
    // The LSN sent in the acknowledgment sufi is in the range VRR <= LSN <= VRH.
    URlcSN mVRR;	// SN of the "last" in-sequence PDU received + 1, meaning
                    // that SN+1 is the first PDU not yet received.
    URlcSN mVRH;	// SN+1 of any PDU received or identified to be missing.
            // See 9.4 how to set it.  A PDU is "identified to be missing"
            // by the POLL SUFI, which sends the VTS from the peer entity.

    // If the PDU size is small we may not be able to fit all the missing
    // blocks in a single status report.  If you only send the oldest
    // status report over and over again, and there is a high PDU loss rate,
    // the total throughput is very slow, especially if the PDU containing
    // the poll bit is lost causing a wait for the mTimerPoll to expire for
    // each transaciton.  To fix that, if we have nothing else to send,
    // continue to send status reports until we have reported all the missing blocks.
    // This variable tells us where we are in the status reports.
    URlcSN mStatusSN;

    URlcSN VRMR() {
        // Maximum acceptable VRR: VRR + Configured_Rx_Window_size
        return addSN(mVRR,mConfig->mConfigured_Rx_Window_Size);
    }

    URlcPdu *mPduRxQ[AmSNS];		// PDU array for reassembly.
    // 11.4.3: Reception of RESET PDU resets all state variables to initial values except VTRST.
    void recvAmInit();	// Happens once.
    public:
    void recvAmReset();	// Happens whenever we get a RESET PDU.
    private:

    URlcAm*parent();
    URlcTransAm*transmitter();
    bool addAckNack(URlcPdu *pdu);
    bool isReceiverOk();

    public:
    URlcRecvAm(URlcConfigAm *wConfig) : URlcRecvAmUm(wConfig), mConfig(wConfig) {
        mRlcid = format("AMR%d",mrbid);
    }

    void rlcWriteLowSide(const BitVector &pdu);
    void text(std::ostream &os);
};

#endif