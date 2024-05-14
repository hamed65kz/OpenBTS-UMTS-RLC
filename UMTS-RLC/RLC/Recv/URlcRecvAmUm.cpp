#include "URlcRecvAmUm.h"
#include "URlcRecvAm.h"
#include "../Tran/URlcTransAm.h"

void URlcRecvAmUm::addUpSdu(ByteVector &payload)
{
    if (mUpSdu == NULL) {
        mUpSdu = new URlcUpSdu(mConfig->mMaxSduSize);
        //RN_MEMLOG(URlcUpSdu,mUpSdu);
        mUpSdu->setAppendP(0);	// Allow appending
    }
    mUpSdu->append(payload);
}

// A gag me special case for LI == 0x7ffc buried in sec 9.2.2.8
void URlcRecvAmUm::ChopOneByteOffSdu(ByteVector &payload)
{
    if (mUpSdu == NULL || mUpSdu->size() < 1) {
        RLCERR("Logic error in the horrible LI=0x7ffc special case");
        return;	// and we are done with that, I guess
    }
    mUpSdu->trimRight(1);	// Chop off the last byte.
}

void URlcRecvAmUm::sendSdu()
{
    rlcSendHighSide(mUpSdu);
    mUpSdu = NULL;
}

void URlcRecvAmUm::discardPartialSdu()
{
    if (mUpSdu) {
        RLCLOG("discardPartialSdu");
        delete mUpSdu;
        mUpSdu = 0;
        // todo: alert other layers.
    }
}

void URlcRecvAmUm::textAmUm(std::ostream &os)
{
    os <<LOGVAR2("mUpSdu.size",(mUpSdu ? mUpSdu->size() : 0));
    os <<LOGVAR(mLostPdu);	// This is UM only, but easier to put in this class.
}

void URlcRecvAmUm::parsePduData(URlcPdu &pdu,
    int headersize,		// 1 for UM, 2 for AM
    bool Eindicator,	// Was the E-bit in the header?
    bool statusOnly)	// If true, process only the piggy-back status, do nothing else.
{
    ByteVector payload = pdu.tail(headersize);
    if (0 == Eindicator) {
		if (statusOnly) { 
			printf("%s", "E Bit not included, statusonly = true\n");
			return; }
        //RLCLOG("parsePduData sn=%d Eindicator=0", pdu.getSN());
		char buf[300] = { 0 };
		sprintf(buf, "parsePduData sn=%d Eindicator=0", pdu.getSN());
		PATLOG(LOG_DEBUG, buf);

        // No LI indicators.  Whole payload is appended to current SDU.
        // mLostPdu is only set in UM mode.
        if (! mLostPdu) {
            addUpSdu(payload);
        }
        return;
    }

    // Crack out the length indicators.
    unsigned licnt = 0;
    unsigned vli[32+1];
    unsigned libytes = mConfig->mUlLiSizeBytes;
    unsigned libits = mConfig->mUlLiSizeBits;
    bool end = 0;
    bool overflow = 0;
    while (!end) {
        if (licnt == 32) {
            overflow = true;
            if (payload.size() < libytes) {
                // Block is complete trash.
                RLCERR("UMTS RLC: Invalid incoming PDU");
                if (! statusOnly) discardPartialSdu();
                return;
            }
        } else {
            vli[licnt++] = payload.getField(0,libits);
        }
        end = (0 == payload.getBit(libits));	// e bit.
        payload.trimLeft(libytes);
        //RLCLOG("parsePduData sn=%d li=%d",
        //    pdu.getSN(),vli[licnt-1]);	// Note: li==127 means padding.
		char buf[300] = { 0 };
		sprintf(buf, "parsePduData sn=%d li=%d", pdu.getSN(), vli[licnt - 1]);
		PATLOG(LOG_DEBUG, buf);
    }

    if (statusOnly) {
        // See if the last li indicates a piggy-backed status is present.
        unsigned lastli = vli[licnt-1];
        if (!(lastli == 0x7ffe || (libytes == 1 && lastli == 0x7e))) {
			printf("%s\n","There is no piggy-backed status present" );
            return;	// There is no piggy-backed status present.
        }
        // keep going and we will handle the piggy back status at the bottom.
    }

    if (overflow) {
        RLCERR("More than 32 segments in an incoming PDU");
    }

    // Use the length indicators to slice up the payload into segments.
    bool start_sdu = false;		// first data byte in pdu starts a new sdu.
    unsigned n = 0;	// index into li fields.

    // Section 11.2.3.1, RLC-UM LI indicators in downlink:
    // first li= 0x7c or 0x7ffc - start of RLC SDU
    // first li= 0x7d or 0x7ffd - complete SDU
    // first li= 0x7ffa - complete SDU - 1 byte

    // Section 9.2.2.8 RLC-UM LI indicators in uplink:
    // 0x7c or 0x7ffc start of RLC SDU.
    // Section 9.2.2.8 RLC-UM LI indicators in downlink:
    // 0x00 or 0x0000 - start of RLC SDU, if no other indication
    // 0x7ffb - prevous pdu was end of RLC SDU - 1 byte

    // Alternative E-bit values:
    // first lie=0x7e or 0x7ffe, 0x7d or 0x7ffd, 0x7ffa

    // 9.2.2.8 Special cases for first length indicator:
    if (vli[0] == 0) {
        // "The previous RLC PDU was exactly filled..."
        start_sdu = true;
        n++;		// And we are finished with this LI field - it didnt actually tell us a length.
    } else if (vli[0] == 0x7ffa) {	// This is only used with the alternative-E-bit config,
                // so we should probably throw an error.
        start_sdu = true;
        // Dont increment n - instead modify vli with the correct length:
        vli[0] = payload.size() - 1;
        payload.trimRight(1);
    } else if (vli[0] == 0x7ffb) {
        // "The previous RLC PDU was one octet short of exactly filling the previous sdu"
        start_sdu = true;
        n++;
        // This horrible special case instructs to chop one byte off the PREVIOUS pdu.  Gag me.
        if (!statusOnly) ChopOneByteOffSdu(payload);
    } else if (vli[0] == 0x7ffc || (libytes == 1 && vli[0] == 0x7c)) {
        // "UMD PDU: The first data octet of this RLC PDU is the first octet
        // of an RLC SDU.  AMD PDU: Reserved."
        start_sdu = true;
        n++;		// And we are finished with this LI field - it didnt actually tell us a length.
    } else if (vli[0] == 0x7ffd || (libytes == 1 && vli[0] == 0x7d)) {
        // "UMD PDU: The first data octet in this RLC PDU is the first octet of an
        // RLC SDU and the last octet in this RLC PDU is the last octet of the same
        // RLC SDU.  AMD PDU: Reserved."
        start_sdu = true;
        vli[0] = payload.size();
        if (licnt != 1) {
            RLCERR("Incoming PDU invalid: LI==0x7ffd but more than one LI");
        }
    } else {
        // If the alternative-E bit, then 0x7ffe and 0x7e have a special meaning
        // in the first position, but we dont use it.
    }

    // Process the piggy-back status and return.
    if (statusOnly) {
        // Scan past all the other li fields to get to the final one.
        for ( ; n < licnt; n++) {
            unsigned lenbytes = vli[n];
            if (lenbytes == 0x7ffe || (libytes == 1 && lenbytes == 0x7e)) {
                // Finally, here it is.
                if (n+1 != licnt) {
                    RLCERR("Incoming piggy-back status indication before end of LI fields");
                    return;
                }

                URlcRecvAm *recv = dynamic_cast<URlcRecvAm*>(this);
                if (recv) {
                    recv->transmitter()->processSUFIs(&payload);
                } else {
                    // The other possibility is that the this object is URlcRecvUm.
                    RLCERR("invalid li=0x7fffe or 0x7e in UM mode");
                }
                return;
            }
            if (lenbytes > payload.size()) {
                RLCERR("Incoming piggy-back status LI size=%d less than PDU length=%d",
                        lenbytes,payload.size());
                return;
            }
            payload.trimLeft(lenbytes);
        }
        return;
    }

    if (start_sdu) {
        if (mLostPdu) { assert(mUpSdu == NULL); }	// this case handled earlier.
        mLostPdu = false;
        // It is an error if mUpSdu is not set, because the sender gave us an LI
        // field that implied that there is an mUpSdu.  But lets not crash...
        if (mUpSdu) sendSdu();
    }

    for ( ; n < licnt; n++) {
        unsigned lenbytes = vli[n];
        //printf("HERE: n=%d libytes=%d lenbytes=%d\n",n,libytes,lenbytes);
        if (lenbytes == 0x7fff || (libytes == 1 && lenbytes == 0x7f) ||
            lenbytes == 0x7ffe || (libytes == 1 && lenbytes == 0x7e)) {
            //printf("THERE: n=%d libytes=%d lenbytes=%d\n",n,libytes,lenbytes);
            // Rest of pdu is padding or a piggy-backed status, which was handled elsewhere.
            payload.trimLeft(payload.size());	// Discard rest of payload.
            if (n+1 != licnt) {
                RLCERR("Incoming PDU padding indication before end of LI fields");
            }
            break;
        }

        // sanity check.
        if (lenbytes > payload.size()) {
            RLCERR("Incoming PDU LI size=%d less than PDU length=%d", lenbytes,payload.size());
            n = licnt;	// End loop after this iteration.
            lenbytes = payload.size();	// Should probably discard this.
        }
        if (!mLostPdu) {
            ByteVector foo(payload.segment(0,lenbytes));	// C++ needs temp variable, sigh.
            addUpSdu(foo);
            sendSdu();
        }
        mLostPdu = false;
        payload.trimLeft(lenbytes);
    }
    if (payload.size()) {
        addUpSdu(payload);	// The left-over is the start of a new sdu to be continued.
    }
}
