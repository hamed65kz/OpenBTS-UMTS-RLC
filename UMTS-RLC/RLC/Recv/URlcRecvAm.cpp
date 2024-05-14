#include "URlcRecvAm.h"
#include "../URlcAm.h"//HKZ
#include "../Tran/URlcTransAm.h" //HKZ
#include "../../Ue/UEInfo.h"

bool URlcRecvAm::isReceiverOk()
{
    // If mVRH < mVRR something horrible is wrong.
    if (deltaSN(mVRH,mVRR) < 0) {
        RLCERR("RLC out of synchronization: mVRR=%d mVRH=%d, doing reset",
                (int)mVRR,(int)mVRH);
        return false;
    }
    return true;
}

// Return true if there are no more status pdus to report after this one.
bool URlcRecvAm::addAckNack(URlcPdu *pdu)
{
    bool lastStatusReport = true;
    bool firstStatusReport;

    // mStatusSN is the mVRR at the time the status was triggered.
    // Bring the mStatusSN up to date in case additional pdus were received
    // between the pdu that triggered the status and now, which
    // is when a pdu is being transmitted with that status.
    firstStatusReport = (deltaSN(mVRR,mStatusSN) >= 0);
    if (firstStatusReport) {
        mStatusSN = mVRR;
    }

    if (mVRR != mVRH) {
        assert(deltaSN(mVRH,mVRR) > 0);  // by definition VRH >= VRR
        assert(mPduRxQ[mVRR] == NULL); // mVRR is last in-sequence pdu received+1
        URlcSN end = mVRH;	// SN+1 of highest pdu known.
        URlcSN sn = mStatusSN;
        // If this happens the RLC is hopelessly out of synchronization aka a bug.
        // We catch this case out readLowSidePdu2() and reset the connection.

        // Gather up ranges of blocks that have not been received.
        // 9.2.2.11.4 List SUFI.  Each one can acknowledge up to 15 missing PDU ranges.
        // The outer while loop stuffs as many of those into the PDU as will fit.
        // Each range low[n] to low[n]+cnt[n] is a series of PDUs that have not been received.
        int n, low[15], cnt[15];
        bool found = false;
        while (sn != end) {
            // The final ACK needs 2 bytes and each LIST SUFI takes 1 + n*2 bytes.
            int maxN = ((int)pdu->sizeRemaining() - 3)/2;
            if (maxN > 15) { maxN = 15; } 	// Max number per LIST SUFI.
            if (maxN == 0) {break;}

            //printf("START maxN=%d sizeBits=%d sizeRemaining=%d\n",maxN,pdu->sizeBits(),pdu->sizeRemaining());
            for (n = 0; n < maxN && sn != end; n++) {
                // Scan forward until we find an unreceived PDU, or are finished.
                // We will already be sitting on an unreceived PDU the first time
                // through this loop (because sn == mVRR) or if the max cnt was reached below.
                for (; sn != end && mPduRxQ[sn]; incSN(sn)) { continue; }
                if (sn == end) { break; }
                low[n] = sn;
                cnt[n] = 1;
                // Scan forward looking for a received PDU.
                for (incSN(sn); sn != end && !mPduRxQ[sn] && cnt[n] < 16; incSN(sn)) {
                    cnt[n]++;	// sn is another adjacent unreceived PDU.
                }
            }
            if (n) {
                // Output the List SUFI.
                // example output: SUFI_LIST=3, n=1, low=0,0,1, cnt=0
                pdu->appendField(SUFI_LIST,4);
                pdu->appendField(n,4);
                char debugmsg[400], *cp = debugmsg;
                //printf("BEFORE sizeBits=%d sizeRemaining=%d\n",pdu->sizeBits(),pdu->sizeRemaining());
                cp += sprintf(cp,"Ack Sufi mVRR=%d mVRH=%d missing:",(int)mVRR,(int)mVRH);
                for (int i = 0; i < n; i++) {
                    // The length field in the sufi is cnt-1, ie, 0 indicates
                    // that only one pdu was missing.
                    pdu->appendField(low[i],12);
                    pdu->appendField(cnt[i]-1,4);
                    cp += sprintf(cp," %d",low[i]);
                    if (cnt[i]>1) { cp += sprintf(cp,"-%d(%d pdus)",low[i]+cnt[i]-1,cnt[i]);}

                    //RLCLOG("AFTER i=%d sizeBits=%d sizeRemaining=%d\n",i,pdu->sizeBits(),pdu->sizeRemaining());
                }
				PATLOG(LOG_DEBUG, debugmsg);
                //RLCLOG("%s",debugmsg);
                found = true;
            }
        }

        if (sn != mVRH) {
            // There are more status reports to transmit.
            lastStatusReport = false;
        }
        mStatusSN = sn;

        if (!found && firstStatusReport) {
            // This can not happen on the first acknack, but may happen
            // on subsequent ones because the pdu that was left to report
            // has been received in the intervening period.
            //RLCLOG("Ack phase error mVRR=%d mVRH=%d but no missing pdus found\n",(int)mVRR,(int)mVRH);
			char buf[300] = { 0 };
			sprintf(buf, "Ack phase error mVRR=%d mVRH=%d but no missing pdus found", (int)mVRR, (int)mVRH);
			PATLOG(LOG_DEBUG, buf);

        }
    } else {
        // Everything is up to date.
        //RLCLOG("Ack Sufi mVRR=%d mVRH=%d all ok\n",(int)mVRR,(int)mVRH);

		char buf[300] = { 0 };
		sprintf(buf, "Ack Sufi mVRR=%d mVRH=%d all ok", (int)mVRR, (int)mVRH);
		PATLOG(LOG_DEBUG, buf);
			

        assert(firstStatusReport && lastStatusReport);
    }
    // 9.2.2.11.2 Ack SUFI is the last SUFI in the Status PDU.
    // The LSN field here specifies that we have received all the PDUS up to
    // LSN except the ones in the Lists we added above.
    // It sets VTA (SN+1 of last positively acked PDU) in the UE transmitter.
    // If this status pdu does not include all the unacked blocks, we
    // are required to set it to VRR.
    // The LSN is confusing because in our case, a binary bit would have sufficed.
    // However, note that if the acks will not fit in a PDU, you are allowed to
    // send them in multiple PDUs, in which case the LSN would be useful.
    // But we are not doing that.
    pdu->appendField(SUFI_ACK,4);
    //pdu->appendField(mStatusSN == mVRH ? mVRH : mVRR,12);
	URlcSN LSN = (firstStatusReport && lastStatusReport) ? mVRH : mVRR;
    pdu->appendField(LSN,12);

	char buf[300] = { 0 };
	sprintf(buf, "Ack Sufi LSN=%d ", (int)LSN);
	PATLOG(LOG_DEBUG, buf);
    return lastStatusReport;
}

void URlcRecvAm::recvAmReset()
{
    URlcRecvAmUm::recvDoReset();
    mVRR = 0;
    mVRH = 0;
    mStatusSN = 0;
    for (int i = 0; i < AmSNS; i++) {
        if (mPduRxQ[i]) { delete mPduRxQ[i]; mPduRxQ[i] = 0; }
    }
}

void URlcRecvAm::text(std::ostream &os)
{
    // There is nothing interesting in URlcRecv to warrant a textRecv() function.
    URlcRecvAmUm::textAmUm(os);

    os <<LOGVAR(mVRR) <<LOGVAR(mVRH) <<LOGVAR2("VRMR",VRMR());
    int cnt=0;
    for (int sns = 0; sns < AmSNS; sns++) {
        if (mPduRxQ[sns]) {
            if (0==cnt++) os <<"\nPduRxQ=";
            os <<"\t"<<LOGVAR(sns); mPduRxQ[sns]->text(os); os <<"\n";
        }
    }
}

void URlcRecvAm::recvAmInit()
{	// Happens once.
    memset(mPduRxQ,0,sizeof(mPduRxQ));
    recvAmReset();
}

// TODO: Should we lock this? In case the MAC manages to send a second
// while we are still doing the first?  Probably so.
void URlcRecvAm::rlcWriteLowSide(const BitVector &pdubits)
{
    ScopedLock lock(parent()->mAmLock);
    int dc = pdubits.peekField(0,1);
    if (dc == 0) { // is it a control pdu?
        URlcPdu pdu1(pdubits,this,"ul am control");
        PduType pdutype = PduType(pdubits.peekField(1,3));

        std::ostringstream foo;
        pdubits.hex(foo);

        /*RLCLOG("rlcWriteLowSide(control,sizebits=%d,pdutype=%d,payload=%s) mVRR=%d",
                pdubits.size(),pdutype,foo.str().c_str(),(int)mVRR);*/
		char buf[300] = { 0 };
		sprintf(buf, "rlcWriteLowSide(control,sizebits=%d,pdutype=%d,payload=%s) mVRR=%d", pdubits.size(), pdutype, foo.str().c_str(), (int)mVRR);
		PATLOG(LOG_DEBUG, buf);
        switch (pdutype) {
            case PDUTYPE_STATUS: {
                transmitter()->processSUFIs(&pdu1);
				printf("%s\n", "recv PDUTYPE_STATUS");
                break;
            }
            case PDUTYPE_RESET: {
                parent()->recvResetPdu(&pdu1);
				printf("%s\n", "recv PDUTYPE_RESET");
                break;
            }
            case PDUTYPE_RESET_ACK: {	// 11.4.4 Reception of RESET ACK PDU
                parent()->recvResetAck(&pdu1);
				printf("%s\n", "recv PDUTYPE_RESET_ACK");
                break;
            }
            default:
                RLCERR("RLC received control block with unknown RLC type=%d",pdutype);
				printf("%s\n", "recv PDUTYPE_RESET_ACK");
                break;
        }
        //delete pdu1;	// formerly I allocated the pdu1, but now not needed.
        return;
    } else {
        // It is a data pdu.
        // Check for poll bit:
        if (pdubits.peekField(URlcPdu::sPollBit,1)) {
            //RLCLOG("Received poll bit at SN %d",(int)pdubits.peekField(1,12));
			char buf[300] = { 0 };
			sprintf(buf, "Received poll bit at SN %d", (int)pdubits.peekField(1, 12));
			PATLOG(LOG_DEBUG, buf);
            transmitter()->mStatusTriggered = true;
            mStatusSN = mVRR;	// This is where we will start the status reports.
        }

        // If we already have a copy we can discard it before we go to the
        // effort of converting it to a ByteVector.
        URlcSN sn = pdubits.peekField(1,12);

       /* if (mUep->mStateChange) {
            int beforesn = sn, beforevrr = mVRR;
            if (sn==0 && (mVRR!=0)) {mUep->reestablishRlcs();}
            LOG(ALERT) << format("stateChange: before %d %d after %d %d",beforesn,beforevrr,(int)sn,(int)mVRR);
        }
        mUep->mStateChange = false;*/

        std::ostringstream foo;
        pdubits.hex(foo);
        /*RLCLOG("rlcWriteLowSide(data,sizebits=%d,sn=%d,payload=%s) mVRR=%d",
                pdubits.size(),(int)sn,foo.str().c_str(),(int)mVRR);*/
		char buf[300] = { 0 };
		sprintf(buf, "rlcWriteLowSide(data,sizebits=%d,sn=%d,payload=%s) mVRR=%d", pdubits.size(), (int)sn, foo.str().c_str(), (int)mVRR);
		PATLOG(LOG_DEBUG, buf);

        if (deltaSN(sn,mVRR) < 0) {
            // The other transmitter sent us a block we have already processed
            // and is no longer in our reception window.
            // This is a common occurrence when there is alot of transmission loss
            // because the other transmitter did not receive our status report
            // informing them that we no longer want this block.
            //RLCLOG("rlcWriteLowSide ignoring block sn=%d less than VRR=%d", (int)sn, (int)mVRR);
			char buf[300] = { 0 };
			sprintf(buf, "rlcWriteLowSide ignoring block sn=%d less than VRR=%d", (int)sn, (int)mVRR);
			PATLOG(LOG_DEBUG, buf);
                        // retransmit status message, otherwise this will go on indefinitely, especially on RACH
            transmitter()->mStatusTriggered = true;
			
            return;
        }

        URlcPdu *pdu2 = new URlcPdu(pdubits,parent(),"ul am data");
     

        // Process piggy-backed status immediately.
        parsePduData(*pdu2,2,pdu2->getAmHE() & 1,true);

        if (mPduRxQ[sn]) {
            // Already received this block.
            // This is a common occurrence if there are many lost pdus
            // because if the status response is lost, the sender will
            // resend a block just to get a poll across.
            // If we were less lazy, we could check that the two blocks match.
            // If the designers had been more clever, they could have used the two blocks
            // to decode the data more securely.
            delete pdu2;
            RLCLOG("rlcWriteLowSide ignoring duplicate block sn=%d", (int)sn);
                        // retransmit status message, otherwise this will go on indefinitely, especially on RACH
                        transmitter()->mStatusTriggered = true;
            return;
        }

        mPduRxQ[sn] = pdu2;

        if (deltaSN(sn,mVRH) >= 0) {
            if (mConfig->mStatusDetectionOfMissingPDU) {
                if (sn != addSN(mVRH,1)) {
                    // We skipped a pdu.  Trigger a status report to inform the other.
                    transmitter()->mStatusTriggered = true;
                }
            }
            mVRH = sn; incSN(mVRH);
        }

        // Now parse any new blocks received.  This is advanceVRR();
        if (sn == mVRR) {
            // Woo hoo!  This is the block we have been waiting for!
            // Advance mVRR over all consecutive blocks that have been received.
            URlcPdu *pdu3;
            while ((pdu3 = mPduRxQ[mVRR])) {
                parsePduData(*pdu3,2,pdu3->getAmHE() & 1,false);
                delete pdu3;
                mPduRxQ[mVRR] = 0;
                incSN(mVRR);
            }
        } else {
            // It is not possible for block mVRR to exist yet.
            assert(mPduRxQ[mVRR] == NULL);
        }
    }
}


URlcAm* URlcRecvAm::parent() { return static_cast<URlcAm*>(this); }
URlcTransAm* URlcRecvAm::transmitter() { return static_cast<URlcTransAm*>(parent()); }
