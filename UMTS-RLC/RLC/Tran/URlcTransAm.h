#ifndef URLCTRANSAM_H
#define URLCTRANSAM_H

#include "../../Utilities/Z100Timer.h"
#include "../Configs/URlcConfigAm.h"

#include "URlcTransAmUm.h"
#include "../Recv/URlcRecvAmUm.h"


class URlcAm;
class URlcRecvAm;
class URlcTransAmUm;
class URlcRecvAmUm;

class URlcTransAm :
    public URlcTransAmUm	// UMTS RLC Acknowledged Mode Transmitter
{
    friend class URlcRecvAm;
    //friend class URlcTransAmUm;
    friend class URlcRecvAmUm;
protected:
    URlcConfigAm *mConfig;
    URlcPdu *readLowSidePdu();
    URlcPdu *readLowSidePdu2();

    // GSM25.322 9.4: AM Send State Variables
    URlcSN mVTS;	// SN of next AMD PDU to be transmitted for the first time.
    URlcSN mVTA;	// SN+1 of last in-sequence positively acknowledged pdu.
                    // This is set by an incoming Ack SUFI 9.2.2.11.2

    // SN of upper edge of transmission window = VTA + VTWS.
    URlcSN VTMS()	{ return addSN(mVTA,mVTWS); }

    UInt_z mVTRST;	// Count number of RESET PDU sent before ack received.  See 11.4.2 and 11.4.5.1.
    // We will not use MRW.
    // unsigned mVTMRW;	// Count number of MRW command transmitted.
    URlcSN mVTWS;	// Window size.  Init to Configured_Tx_Window_size.
            // (pat) The window size is how many unacked blocks you can send before stalling.
            // Warning will robinson: it can theoretically be set up to SN-1,
            // which if allowed would cause deltaSN(), etc to fail.

    Z100Timer mTimer_Poll;		// Set when a poll is sent, and stopped when the poll is answered.
    URlcSN mTimer_Poll_VTS;	// Described in 25.322 9.5 paragraph a.
    //unsigned mTimer_Poll_SN;	// The value of VTS at the time poll was sent; timer is
            // stopped if ack is received for this and preceding SN.
    Z100Timer mTimer_Poll_Prohibit;
    //Z100 mTimer_Discard;			unimplemented
    Z100Timer mTimer_Poll_Periodic;	// How often to poll.
    //Z100 mTimer_Status_Prohibit;	// How often to send unsolicited (unpolled) status reports.
    //Z100 mTimer_Status_Periodic;	unimplemented
    Z100Timer mTimer_RST; // start when RESET PDU sent, stop when RESET ACK received.
                    // Upon expiry, resend RESET PDU with same RSN
    //Z100 Timer_MRW - Resend MRW SUFI when expired.  We wont use MRW; it is unneeded.

    //URlcRecvAm *mRecv;	// Pointer to the paired receiving entity.

    URlcPdu* mPduTxQ[AmSNS];		// PDU array, saved for possible retransmission.
                                    // Note that only data pdus go in here, not control.

    // Variables pat added:
    bool mNackedBlocksWaiting;	// True if mNackVS is valid.
    URlcSN mVSNack;		// Next nacked block to be retransmitted.

    bool mPollTriggered;
    bool mStatusTriggered;
    bool mResetTriggered;	// This just triggers it.  An in-progress reset is indicated by mTimer_RST.active()

    unsigned mResetTransRSN;		// RSN value we sent in our last reset pdu.
    unsigned mResetAckRSN;			// RSN value of last received reset pdu, saved to put in RESET_ACK message.
    //unsigned mResetTransCount;		// Total Number of resets transmitted.
    unsigned mResetRecvCount;		// Total Number of resets received, ever.
    //unsigned mResetAckRecvCount;	// Total number of reset ack received.
    bool mSendResetAck;

    unsigned mVTPDUPollTrigger;		// Next trigger for a poll if Poll_PDU option, or 0.
    unsigned mVTSDUPollTrigger;		// Next trigger for a poll if Poll_SDU option, or 0.

    public:
    bool resetInProgress() { return mTimer_RST.active(); }
    void transAmReset();	// Happens whenever we get a reset PDU.
    void transAmInit();		// Happens once.
    private:
    URlcAm*parent();
    URlcRecvAm*receiver();

    bool stalled();
    void setNAck(URlcSN sn);	// Set the nack indicator for queued block with this sequence number.
    URlcPdu *getDataPdu();
    URlcPdu *getResetPdu(PduType type);
    URlcPdu *getStatusPdu();
    void advanceVTA(URlcSN newvta);
    void advanceVS(bool);
    void processSUFIs(ByteVector *vec);
    void processSUFIs2(ByteVector *vec, size_t rp);
    bool IsPollTriggered();
    unsigned rlcGetDlPduSizeBytes() { return mConfig->mDlPduSizeBytes; }

    public:
    // This class is not allocated alone; it is part of URlcAm.
    URlcTransAm(URlcConfigAm *wConfig) :
        URlcTransAmUm(wConfig),
        mConfig(wConfig)
        {
            mRlcid = format("AMT%d",mrbid);
        }
    void text(std::ostream &os);
    void triggerReset() { mResetTriggered = true; }	// for testing
};
#endif