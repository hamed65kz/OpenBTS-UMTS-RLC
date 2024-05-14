#include "URlcTransAm.h"
#include "../URlcAm.h"
#include "../../Utilities/Logger.h"

URlcAm* URlcTransAm::parent() { return dynamic_cast<URlcAm*>(this); }//HKZ dynamic cast
URlcRecvAm* URlcTransAm::receiver() { return static_cast<URlcRecvAm*>(parent()); }


URlcPdu *URlcTransAm::getDataPdu()
{
    if (pdusFinished()) { return NULL;	} // No data waiting in the queue.

    URlcPdu *result = new URlcPdu(mConfig->mDlPduSizeBytes,parent(),"dl am");
    result->fill(0,0,2);	// Be safe and clear out the header.
    result->setAmDC(1);		// Data pdu.
    // The spec says nothing about AM buffer overflow behavior except when
    // in "explicit signaling" modes.  If these are TCP packets, it is ok
    // to just drop them, so that's what we'll do.  Pretty easy:
    // just ignore the fillPduData returned result.
    //LOG(INFO) << "fPD time " << gNodeB.clock().get();
    fillPduData(result,2,0);
        //LOG(INFO) << "fPD done " << gNodeB.clock().get();
    result->setAmSN(mVTS);
    result->setAmP(0);	// Until we know better.
    //RLCLOG("getDataPdu VTS=%d,VTA=%d bytes=%d header=%s dc=%d sn=%u",
    //    (int)mVTS,(int)mVTA,result->sizeBytes(),result->segment(0,2).hexstr().c_str(),
    //    result->getBit(0),(int)result->getField(1,12));

	char buf[300] = { 0 };
	sprintf(buf, "getDataPdu VTS = %d, VTA = %d bytes = %d header = %s dc = %d sn = %u", (int)mVTS, (int)mVTA, result->sizeBytes(), result->segment(0, 2).hexstr().c_str(),
		result->getBit(0), (int)result->getField(1, 12));
	PATLOG(LOG_DEBUG, buf);

    if (mPduTxQ[mVTS]) {
        RLCERR("RLC-AM internal error: PduTxQ at %d not empty",(int)mVTS);
        delete mPduTxQ[mVTS];
    }
    mPduTxQ[mVTS] = result;
    incSN(mVTS);
    return result;
}

// Return a reset or reset ack pdu depending on type.
URlcPdu *URlcTransAm::getResetPdu(PduType type)
{
    RLCLOG(type == PDUTYPE_RESET ? "Sending reset pdu" : "Sending reset_ack pdu");
    URlcPdu *pdu = new URlcPdu(mConfig->mDlPduSizeBytes,parent(),"dl reset");
    pdu->fill(0);
    pdu->setAppendP(0);
    pdu->appendField(0,1);		// DC == 0 for control pdu;
    pdu->appendField(type,3);
    pdu->appendField(type == PDUTYPE_RESET ? mResetTransRSN : mResetAckRSN,1);
    pdu->appendField(0,3);	// R1, reserved field;
    pdu->appendField(parent()->mDLHFN,20);
    pdu->appendZero();
    return pdu;
}


URlcPdu *URlcTransAm::getStatusPdu()
{
    URlcPdu *pdu = new URlcPdu(mConfig->mDlPduSizeBytes,parent(),"dl status");
    pdu->fill(0);
    pdu->setAppendP(0);
    pdu->appendField(0,1);		// DC == 0 for control pdu;
    pdu->appendField(PDUTYPE_STATUS,3);
    // Now the sufis.
    if (receiver()->addAckNack(pdu)) {
        mStatusTriggered = false;
    }

    // Zero fill to a byte boundary:
    pdu->appendZero();
    return pdu;
}

// The SUFI received by the receiver advances VTA in the transmitter.
// PDUs up to sn (or sn+1?) have been acknowledged by the peer entity.
void URlcTransAm::advanceVTA(URlcSN newvta)
{
    for ( ; deltaSN(mVTA,newvta) < 0; incSN(mVTA)) {
        if (mPduTxQ[mVTA]) { delete mPduTxQ[mVTA]; mPduTxQ[mVTA] = NULL; }
    }
}

// The status pdu with the SUFIs is received by the receiving entity but the information
// applies to and is processed by the transmitting entity.
// Side effect: if we receive an acknowledgement set mNackedBlocksWaiting and
// mVSNack to the oldest negatively acknowledged block, if any.
// SUFI 25.322 9.2.2.11
void URlcTransAm::processSUFIs2(
    ByteVector *vec,	// May come from a status pdu or a piggy-backed status pdu.
    size_t rp)		// Bit position where the sufis start in vec; they are not byte-aligned.
{
    unsigned i, j;
    URlcSN sn;
    URlcSN newva = mVTA;	// SN of oldest block nacked by this pdu.
    bool newvaValid = false;
   /* RLCLOG("Sufis before: VTS=%d VTA=%d VSNack=%d NackedBlocksWaiting=%d",
        (int)mVTS,(int)mVTA,(int)mVSNack,mNackedBlocksWaiting);*/
	char buf[300] = { 0 };
	sprintf(buf, "Sufis before: VTS=%d VTA=%d VSNack=%d NackedBlocksWaiting=%d", (int)mVTS, (int)mVTA, (int)mVSNack, mNackedBlocksWaiting);
	PATLOG(LOG_DEBUG, buf);

    while (1) {
        SufiType sufitype = (SufiType) vec->readField(rp,4);
        switch (sufitype) {
        case SUFI_NO_MORE:
            return;
        case SUFI_WINDOW:
            // The minimum and maximum values are set by higher layers, which is us,
            // so this should not happen if we set them the same.
            sn = vec->readField(rp,12);
            mVTWS = sn;
            continue;
        case SUFI_ACK: {
            URlcSN lsn = vec->readField(rp,12);
            if (!newvaValid || deltaSN(lsn,newva) <= 0) {
                advanceVTA(lsn);
            } else {
                advanceVTA(newva);
            }
            if (mNackedBlocksWaiting) advanceVS(true);

           /* RLCLOG("Sufis after: lsn=%d newvaValid=%d newva=%d VTS=%d VTA=%d VSNack=%d NackedBlocksWaiting=%d",
                (int)lsn,newvaValid,(int)newva,(int)mVTS,(int)mVTA,(int)mVSNack,mNackedBlocksWaiting);*/
			char buf[300] = { 0 };
			sprintf(buf, "Sufis after: lsn=%d newvaValid=%d newva=%d VTS=%d VTA=%d VSNack=%d NackedBlocksWaiting=%d", (int)lsn, newvaValid, (int)newva, (int)mVTS, (int)mVTA, (int)mVSNack, mNackedBlocksWaiting);
			PATLOG(LOG_DEBUG, buf);

            return;	// SUFI_ACK is always the last field.
        }
        case SUFI_LIST: {
            unsigned numpairs = vec->readField(rp,4);
            if (numpairs == 0) {
                RLCERR("Received SUFI LIST with length==0");
                return;	// Give up.
            }
            for (i = 0; i < numpairs; i++) {
                sn = vec->readField(rp,12);
                if (i == 0) { newva = minSN(newva,sn); newvaValid=true; }
                unsigned nackcount = vec->readField(rp,4) + 1;
                for (j = 0; j < nackcount; j++, incSN(sn)) {
                    setNAck(sn);
                }
            }
            RLCLOG("received SUFI_LIST n=%d",numpairs);
            continue;
        }
        case SUFI_BITMAP: {
            // There is a note in 9.2.2.11.5:
            // "NOTE: The transmission window is not advanced based on BITMAP SUFIs"
            // So why is this note in the BITMAP sufi and not in the LIST and RLIST sufis, you ask?
            // It is because all acks are negative, and instead of using positive acks,
            // UMTS sends an "ACK" sufi that advances VTA, which implicitly positively
            // acks all passed over blocks.  Therefore you are not to construe
            // the bitmap entries with the bit set as a positive ack, you are only
            // to pay attention to the negative acks in the bitmap.
            unsigned maplen = 8*(vec->readField(rp,4) + 1);	// Size of bitmap in bits
            sn = vec->readField(rp,12);
            for (i = 0; i < maplen; i++, incSN(sn)) {
                int bit = vec->readField(rp,1);
                if (bit == 0) {
                    newva = minSN(newva,sn);
                    newvaValid = true;
                    setNAck(sn);
                }
            }
            continue;
        }
        case SUFI_RLIST: {	// The inventor of this was on drugs.
            unsigned numcw = vec->readField(rp,4);
            sn = vec->readField(rp,12);
            setNAck(sn);
            newva = minSN(newva,sn);
            newvaValid = true;
            incSN(sn);
            unsigned accumulator = 0;
            bool superSpecialErrorBurstIndicatorFlag = false;
            for (i = 0; i < numcw; i++) {
                unsigned cw = vec->readField(rp,4);
                if (cw == 1) {
                    if (accumulator) {
                        RLCERR("Received invalid SUFI RLIST with incorrectly placed error burst indicator");
                        return;	// Give up.
                    }
                    superSpecialErrorBurstIndicatorFlag = true;
                    continue;
                } else {
                    accumulator = (accumulator << 3) | (cw>>1);
                    if (cw & 1) {
                        if (superSpecialErrorBurstIndicatorFlag) {
                            while (accumulator--) {
                                setNAck(sn);
                                incSN(sn);
                            }
                        } else {
                            // Gag me, the spec is not clear what the distance really is:
                            // "the number ... represents a distance between the previous indicated
                            // erroneous AMD PDU up to and including the next erroneous AMD PDU."
                            sn = addSN(sn,accumulator-1);
                            setNAck(sn);
                            incSN(sn);
                        }
                        superSpecialErrorBurstIndicatorFlag = false;
                        accumulator = 0;
                    }
                }
            }
            if (superSpecialErrorBurstIndicatorFlag) {
                // It is an error. We are supposed to go back and undo everything
                // we just did, but heck with that.
                RLCERR("Received invalid SUFI RLIST with trailing error burst indicator");
                continue;
            }
            continue;
        }
        case SUFI_MRW: {		// We dont implement this, but parse over it anyway.
            unsigned numMRW = vec->readField(rp,4);
            if (numMRW == 0) numMRW = 1;	// special case
            for (i = 0; i < numMRW; i++) {
                vec->readField(rp,12);	// MRW
            }
            vec->readField(rp,4);	// N_length
            continue;
        }
        case SUFI_MRW_ACK: {	// We dont implement, but parse over it anyway.
            /*unsigned numMRWAck =*/ vec->readField(rp,4);
            vec->readField(rp,4);	// N
            vec->readField(rp,12);	// SN_ACK
            continue;
        }
        case SUFI_POLL: {
            // This can only be used if "flexible RLC PDU size" is configured,
            // and I dont think it will be.
            sn = vec->readField(rp,12);
            // TODO...
            continue;
        }
        default:
            RLCERR("Received invalid SUFI type=%d",sufitype);
            return;
        } // switch
    } // while
}

void URlcTransAm::processSUFIs(ByteVector *vec)
{
    size_t rp = 4;
    processSUFIs2(vec,rp);

    if (mConfig->mPoll.mTimerPoll && mTimer_Poll.active()) {
        // Reset Timer_Poll exactly as per 25.322 9.5 paragraph a.
        // The purpose of the poll timer is to positively insure that
        // the poll that occurred at mTimer_Poll_VTS gets through.
        // This complicated check is to prevent an in-flight status report from
        // turning of the Poll timer prematurely.  It is over-kill; I think
        // we could just test mNackedBlocksWaiting, which forces a re-poll.
        URlcPdu *pdu=NULL;
        assert(mTimer_Poll_VTS < AmSNS);
        if (deltaSN(mVTA,mTimer_Poll_VTS) >= 0 ||
            ((pdu = mPduTxQ[mTimer_Poll_VTS]) && pdu->mNacked)) {
           /* RLCLOG("Timer_Poll.reset VTA=%d Timer_Poll_VTS=%d nacked=%d",
                (int)mVTA,(int)mTimer_Poll_VTS,pdu?pdu->mNacked:-1);*/
			char buf[300] = { 0 };
			sprintf(buf, "Timer_Poll.reset VTA=%d Timer_Poll_VTS=%d nacked=%d", (int)mVTA, (int)mTimer_Poll_VTS, pdu ? pdu->mNacked : -1);
			PATLOG(LOG_DEBUG, buf);
            mTimer_Poll.reset();
        }
    }
}

// Move mVSNack forward to the next negatively acknowledged block, if any.
// If none, reset mNackedBlocksWaiting.
// Apparently we dont resend blocks awaiting an acknack.
// If fromScratch, start over from the beginning.
void URlcTransAm::advanceVS(bool fromScratch)
{
    if (fromScratch) {
        mVSNack = mVTA;
    } else {
        incSN(mVSNack);	// Skip nacked block we just sent.
    }
    for ( ; deltaSN(mVSNack,mVTS) < 0; incSN(mVSNack)) {
        URlcPdu *pdu = mPduTxQ[mVSNack];
        if (! pdu) { continue; } // block was acked and deleted.
        if (pdu->mNacked) return;
    }
    // No more negatively acknowledged blocks at the moment.
    // But note there may be lots of blocks that are UnAcked.
    mNackedBlocksWaiting = false;
}

bool URlcTransAm::IsPollTriggered()
{
    if (mPollTriggered) return true;

    // 9.7.1 case 4.
    if (mConfig->mPoll.mPollPdu) {
        if (deltaSN(mVTPDU,mVTPDUPollTrigger) >= 0) {
            //RLCLOG("PollPdu triggered, VTPDU=%d trig=%d",(int)mVTPDU,(int)mVTPDUPollTrigger);
			char buf[300] = { 0 };
			sprintf(buf, "PollPdu triggered, VTPDU=%d trig=%d", (int)mVTPDU, (int)mVTPDUPollTrigger);
			PATLOG(LOG_DEBUG, buf);
            mPollTriggered = true;
            mVTPDUPollTrigger = mVTPDU + mConfig->mPoll.mPollPdu;
        }
    }

    // 9.7.1 case 5.
    // The poll is meant to be sent after the entire SDU has been sent
    // so we get acknowledgement for the whole thing, so if the
    // special case mLILeftOver is still outstanding, wait for that.
    if (mConfig->mPoll.mPollSdu) {
        int diff = deltaSN(mVTSDU,mVTSDUPollTrigger);
        if (diff > 0 || (diff == 0 && mLILeftOver == 0)) {
            //RLCLOG("PollSdu triggered VTSDU=%d trig=%d",(int)mVTSDU,(int)mVTSDUPollTrigger);
			char buf[300] = { 0 };
			sprintf(buf, "PollSdu triggered VTSDU=%d trig=%d", (int)mVTSDU, (int)mVTSDUPollTrigger);
			PATLOG(LOG_DEBUG, buf);
            mPollTriggered = true;
            mVTSDUPollTrigger = mVTSDU + mConfig->mPoll.mPollSdu;
        }
    }

    // 9.7.1 case 7.
    if (mConfig->mPoll.mTimerPollPeriodic) {
        if (mTimer_Poll_Periodic.expired()) {
            RLCLOG("TimerPollPeriodic triggered");
            mPollTriggered = true;
            mTimer_Poll_Periodic.set();
        }
    }

    // 9.7.1 case 3.  Described more thoroughly in 9.5
    if (mConfig->mPoll.mTimerPoll) {
        if (mTimer_Poll.expired()) {
            RLCLOG("TimerPoll triggered");
            mPollTriggered = true;
            // This timer is reset when we actually send the poll.
        }
    }
    return mPollTriggered;
}

bool URlcTransAm::stalled()
{
    // FIXME: This does not work unless mVTWS is less than mSNS/2.
    return deltaSN(mVTS,mVTA) >= mVTWS;
}

URlcPdu *URlcTransAm::readLowSidePdu2()
{
    if (mRlcState != RLC_RUN) { return NULL; }

    if (mSendResetAck) {
        mSendResetAck = false;
        return getResetPdu(PDUTYPE_RESET_ACK);
    }

    // Did we initiate a reset procedure, and are awaiting the Reset_ack?
    if (resetInProgress()) {
        RLCLOG("Reset in progress remaining=%d",(int)mTimer_RST.remaining());
        // Is the timer expired?
        if (mTimer_RST.expired()) {
            // Resend the same reset.
            // Note that the default max_RST for SRBs is just 1,
            // so if a reset is lost, the channel is abandoned.
            mResetTriggered = true;	// handled below.
        } else {
            return NULL;	// Nothing to send; waiting on reset ack.
        }
    }

    // If VRH and VRR become inverted, something terrible is wrong.
    // Send a reset.  Also see addAckNack().
    if (!receiver()->isReceiverOk()) {
        mResetTriggered = true;
    }

    // 11.4.2 Reset Initiation
    if (mResetTriggered) {
        RLCLOG("reset triggered VTRST=%d max=%d",(int)mVTRST,mConfig->mMaxRST);
        mResetTriggered = 0;
        // We dont flush mPDUs and discard partial sdus until we get the reset_ack,
        // although I'm not sure why the timing would matter.
        mVTRST++;
        if (mVTRST > mConfig->mMaxRST) {
            RLCERR("too many resets, connection disabled");
            // Too many resets.  Give up.
            mTimer_RST.reset();	// Finished with the timer.
            mRlcState = RLC_STOP;
            // TODO: We may want to flush the SDU buffer.
            // TODO: clause 11.4.4a, which is send unrecoverable error to upper layer.
            return NULL;
        }
        mTimer_RST.set(mConfig->mTimerRSTValue);
        // We need to increment RSN between resets, but we cant really do it
        // after we send the RESET_ACK in recvResetAck,
        // because we might get multiple ones of those.
        // It is easier to increment it before starting a new reset procedure.
        return getResetPdu(PDUTYPE_RESET);
    }

    // Optional here: timer based status transfer;  If mTimer_status_periodic expired,
    // set mStatusTriggered.

    // mStatusTriggered may be triggered when the receiver notices a missing pdu,
    // or when requested by a poll (that indicates PDU was last in senders buffer,
    // or optionally by timer in sender), or optionally by timer in receiver.
    // mStatusTriggered will not be reset until we have transmitted
    // enough pdus for a complete status report.
    if (mStatusTriggered) {
        // We are allowed to piggy-back the status, but not implemented for downlink.
        return getStatusPdu();
    }

    // Find a pdu to send.  The pdu variable may only be a data pdu, because
    // at the end we will set/unset the poll bit, which is only valid in data pdus.
    URlcPdu *pdu = NULL;

    // Section 11.3.2 Transmission of AMD PDU: this section is confusing.
    // Essentially it is establishing the priority of PDUs to be sent, which is:
    //	1. negatively acknowledged PDUs.
    //	2. new PDUs.
    // 	3. I dont see where it says anything about resending pdus before
    //		receiving negative ack.
    // If a status report is triggered, it can be sent in a stand-alone status report,
    // or it can be piggy-backed onto a previously sent PDU.
    // If the Configured_Tx_Window_Size >= 2048 (half the sequence space) then
    // you may only resend the most recently sent PDU.

    if (mNackedBlocksWaiting) {
        // Send this negatively acknowledged pdu.
        // TODO: If we support piggy-backed status, that needs to be fixed here too.
        pdu = mPduTxQ[mVSNack];
        pdu->mNacked = false;
        // Unset the poll bit in case it had been set on the previous transmission.
        pdu->setAmP(false);
        advanceVS(false);
        // 9.7.1 case 2: If the AMD PDU is the last of the AMD PDUs scheduled for retransmission... poll.
        if (!mNackedBlocksWaiting && mConfig->mPoll.mLastRetransmissionPduPoll) {
            mPollTriggered = true;
        }
    } else if (stalled()) {
        RLCLOG("Stalled VTS=%d VTA=%d",(int)mVTS,(int)mVTA);
        // No new data to send, but go to maybe_poll because if the
        // previous poll was prohibited by the mTimerPollProhibit, it may
        // have expired now and we can finally send the poll.
        goto maybe_poll;
    } else if ((pdu = getDataPdu())) {
        // 9.7.1 case 1: If it is the last PDU available... poll.
        if (mConfig->mPoll.mLastTransmissionPduPoll && pdusFinished()) {
            mPollTriggered = true;
        }
    } else {
        // No new pdus to send, but according to 9.7.1, we may need to send
        // a poll anyway so dont return yet.
    }

    if (pdu && ++pdu->mVTDAT >= mConfig->mMaxDAT) {
        // 25.322 11.3.3a: Reached maximum number of attempts.
        // Note that the documentation of the option names does not exactly match
        // the names in 25.331 RRC 10.3.4.25 Transmission RLC Discard IE.
        RLCLOG("pdu %d exceeded VTDAT=%d, discarded",pdu->getAmSN(),pdu->mVTDAT);
        switch (mConfig->mRlcDiscard.mSduDiscardMode) {
        default:
            RLCERR("Unsupported RLC discard mode configured:%d",
                    (int)mConfig->mRlcDiscard.mSduDiscardMode);
            // fall through.
        case TransmissionRlcDiscard::NoDiscard:
            // "No discard" means no explicit discard of just this SDU using MRW sufis;
            // instead we just reset the whole connection.
            mResetTriggered = true;
            return readLowSidePdu2();	// recur to send reset pdu
        }
    }

    // Set the poll bit if any poll triggers as per 9.7.1, starting at:
    // "When the Polling function is triggered, the Sender shall..."
    maybe_poll:
    if (mConfig->mPoll.mTimerPoll) {
        if (mTimer_Poll.active()) {
            RLCLOG("Timer_poll active=%d remaining=%ld",mTimer_Poll.active(),mTimer_Poll.remaining());
        }
    }
    if (IsPollTriggered()) {
        if (mConfig->mPoll.mTimerPollProhibit && !mTimer_Poll_Prohibit.expired()) {
            // Delay polling.
        } else {
            if (pdu == NULL) {
                // 9.7.1, near top of page 54:
                // "If there is one or more AMD PDUs to be transmitted or
                // there are AMD PDUs not acknowledged by the Receiver:"
                // Note carefully: It does not say "negatively acknowledged" blocks,
                // which would be a test of mNackedBlocksWaiting; rather we resend
                // the poll request if any blocks have not been acknowledged,
                // which occurs if mVTS > mVTA.
                if (mVTS != mVTA) {
                    // We need to resend a PDU just to set the poll bit.
                    // This is particularly important for the Timer_Poll.
                    // Consider what happens if the PDU carrying the poll bit is lost;
                    // then the RLC on the other end does not respond, the
                    // Timer_Poll expires, and we get to here now.
                    // 11.3.2 says if we need to set a poll and have nothing
                    // to send, resent pdu[mVTS-1]
                    URlcSN psn = addSN(mVTS,-1);
                    pdu = mPduTxQ[psn];
                    if (pdu == NULL) {
                        RLCERR("internal error: pdu[mVTS-1] is missing");
                        return NULL;
                    }
                } else {
                    return NULL;
                }
            }

            // Send the poll.
            mPollTriggered = false;
            pdu->setAmP(true);
            RLCLOG("Poll Requested at sn=%d",pdu->getSN());
            if (mConfig->mPoll.mTimerPollProhibit) { mTimer_Poll_Prohibit.set(); }
            if (mConfig->mPoll.mTimerPoll) {
                mTimer_Poll.set();
                mTimer_Poll_VTS = mVTS;
            }
        }
    }

    // If sending a data pdu, it is saved in mPduTxQ, so we have to send a copy
    // for the caller to delete.
    if (pdu) {
        pdu = new URlcPdu(pdu);
    }

    return pdu;
}

URlcPdu *URlcTransAm::readLowSidePdu()
{
    ScopedLock lock(parent()->mAmLock);
    URlcPdu *pdu = readLowSidePdu2();
    if (pdu) {
        bool dc = pdu->getBit(0);
        if (dc) {
            // Data pdu:
            //RLCLOG("readLowSidePdu dc=data sn=%d poll=%d header=%s",
            //    (int)pdu->getField(1,12),
            //    (int)pdu->getField(URlcPdu::sPollBit,1),
            //    pdu->segment(0,2).hexstr().c_str());


			char buf[300] = { 0 };
			sprintf(buf, "readLowSidePdu dc=data sn=%d poll=%d header=%s", (int)pdu->getField(1, 12),
				(int)pdu->getField(URlcPdu::sPollBit, 1),
				pdu->segment(0, 2).hexstr().c_str());
			PATLOG(LOG_DEBUG, buf);

        } else {
            // Control pdu: the sn is not applicable.
            //RLCLOG("readLowSidePdu dc=control header=%s", pdu->segment(0,2).hexstr().c_str())
			char buf[300] = { 0 };
			sprintf(buf, "readLowSidePdu dc=control header=%s", pdu->segment(0, 2).hexstr().c_str());
			PATLOG(LOG_DEBUG, buf);
        }
    }
    return pdu;
}

void URlcTransAm::transAmReset()
{
    RLCLOG("transAmReset");
    URlcTransAmUm::transDoReset();
    mVTS = 0;
    mVTA = 0;
    mVTWS = mConfig->mConfigured_Tx_Window_Size;
    mVTPDUPollTrigger = mConfig->mPoll.mPollPdu;
    mVTSDUPollTrigger = mConfig->mPoll.mPollSdu;
    mPollTriggered = mStatusTriggered = mResetTriggered = false;
    mNackedBlocksWaiting = false;
    mVSNack = 0;
    mSendResetAck = false;
    for (int i = 0; i < AmSNS; i++) {
        if (mPduTxQ[i]) { delete mPduTxQ[i]; mPduTxQ[i] = 0; }
    }
    // mResetTransRSN is explicitly not reset.
    // mVTRST is explicitly not reset.
    // mVTMRW = 0;	currently unused

    //if (mConfig->mTimerRSTValue) mTimer_RST.reset(); // <- done by caller:
    // 25.322 11.4.1: Reset does not reset mTimerPollPeriodic
    //if (mConfig->mPoll.mTimerPollPeriodic) mTimer_Poll_Periodic.reset();
    if (mConfig->mPoll.mTimerPollProhibit) mTimer_Poll_Prohibit.reset();
    if (mConfig->mPoll.mTimerPoll) mTimer_Poll.reset();
}

void URlcTransAm::text(std::ostream&os)
{
    URlcTrans::textTrans(os);
    URlcTransAmUm::textAmUm(os);

    os <<LOGVAR(mVTS) <<LOGVAR(mVTA) <<LOGVAR(mVTWS)
        <<LOGVAR(mPollTriggered) <<LOGVAR(mStatusTriggered) <<LOGVAR(mResetTriggered)
        <<LOGVAR(mNackedBlocksWaiting) <<LOGVAR(mSendResetAck)
        <<LOGVAR(mResetTransRSN) <<LOGVAR(mResetAckRSN) <<LOGVAR(mResetRecvCount);
    int cnt = 0;
    for (int sns = 0; sns < AmSNS; sns++) {
        if (mPduTxQ[sns]) {
            if (0==cnt++) os <<"\nPduTxQ=";
            os <<"\t"<<LOGVAR(sns); mPduTxQ[sns]->text(os); os<<"\n";
        }
    }
}


void URlcTransAm::transAmInit()
{
    mResetTransRSN = 0;
    mResetAckRSN = 0;
    mResetRecvCount = 0;
    memset(mPduTxQ,0,sizeof(mPduTxQ));
    mTimer_RST.configure(mConfig->mTimerRSTValue);
    mTimer_Poll_Periodic.configure(mConfig->mPoll.mTimerPollPeriodic);
    mTimer_Poll_Prohibit.configure(mConfig->mPoll.mTimerPollProhibit);
    mTimer_Poll.configure(mConfig->mPoll.mTimerPoll);
    // etc...
    transAmReset();
}

// Set the nack indicator for queued block with this sequence number.
void URlcTransAm::setNAck(URlcSN sn)
{
    assert(sn >= 0 && sn < AmSNS);
    if (URlcPdu *pdu = mPduTxQ[sn]) {
        pdu->mNacked = true;
        mNackedBlocksWaiting = true;
        RLCLOG("setNack %d pdu->sn=%d",(int)sn,pdu->getAmSN());
    } else {
        RLCLOG("setNack %d MISSING PDU!",(int)sn);
    }
}
