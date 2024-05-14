#include "URlcTransAmUm.h"
#include "../../Utilities/Logger.h"

// Common routine for Am and Um modes to fill the data part of the downlink pdu.
// The Am and Um pdus have different header sizes, which is passed in pduHeaderSize.
// The result is a discard notification which is true if UM sdus were discarded.
// The 'alternate interpretation of HE' is a wonderful thing that indicates
// the end of the SDU and gets rid of many of the special cases below,
// but unfortunately it only exists in AM, and we still need the special
// cases for UM, so I did not implement it.
bool URlcTransAmUm::fillPduData(URlcPdu *result,
                                unsigned pduHeaderSize,
                                bool *pNewSdu)	// Set if this pdu is the start of a new sdu.
{
    ScopedLock lock(mQLock);

    // Step one: how many sdus can we fit in this pdu?
    // remaining = How many bytes left in output PDU.
    unsigned remaining = result->size() - pduHeaderSize;

    bool mSduDiscarded = false;	// Set if discarded SDU is detected.

    unsigned sducnt = 0;		// Number of whole or partial sdus sent.
    unsigned sdufinalbytes = 0;	// If non-zero, final sdu is split and this number of bytes sent.
    unsigned vli[100];	// We will gather up the LI indicators as we go.
    unsigned libytes = mConfig->mDlLiSizeBytes;
    int licnt = 0;

    URlcDownSdu *sdu = mSplitSdu ? mSplitSdu : mSduTxQ.front();
    bool newSdu = sdu && !mSplitSdu;	// We are starting a new sdu.
    if (pNewSdu) {*pNewSdu = newSdu;}

    if (sdu == NULL) {
        // This special case occurs if the previous sdu exactly filled the
        // the pdu, but we need an indication in the next pdu (which is the
        // one we are currently creating) to mark the end of that sdu,
        // but the incoming sdu queue is empty.
        // We need to output one pdu with only li flags, no data, to notify
        // the receiver on the other end of the end of the sdu.
        if (mLILeftOver) {
            remaining -= libytes;	// For the LI field we are about to add.
            vli[licnt++] = (mLILeftOver == 1) ? 0 : 0x7ffb;
            mLILeftOver = 0;
        } else {
            assert(0);	// Caller prevents this by checking pdusFinished first.
        }
        result->mDescr = string("li_only");
    } else {
        result->mDescr += sdu->mDescr;
    }

    URlcDownSdu *sdunext = NULL;
    for ( ; sdu; sdu = sdunext) {
        if (! mConfig->mIsSharedRlc) { // SharedRlc only sends one SDU at a time.
            sdunext = (sdu == mSplitSdu) ? mSduTxQ.front() : sdu->next();
        }
        if (sdu->mDiscarded) {
            // SDU was discarded, possibly because transmission buffer full.
            if (mRlcMode == URlcModeUm) {
                // Perform function of 11.2.4.3: SDU discard without explicit signaling.
                // Also applies to the case of SDU discard not configured when transmission buffer
                // is full and a partially sent SDU is discarded as per 9.7.3.5, however,
                // that depends on whether we decide to discard in-progress sdus or not.
                // I am going to not do that, since the in-progress sdu probably represents
                // usable data on the other end.
                if (mSduDiscarded == false) {
                    mSduDiscarded = true;
                    if (mConfig->mRlcDiscard.mSduDiscardMode != TransmissionRlcDiscard::NotConfigured) {
                        if (sducnt) {
                            // We need to inform the peer where the missing SDU occurred
                            // so we cant put back-to-back SDUs into the PDU with
                            // the missing SDU (not) between them, so stop now.
                            // And leave the discarded SDU here so we will see it next time.
                            break;
                        } else {
                            // Force fillPduData to add an extra first Length Indicator
                            // to indicate start of SDU.
                            mLILeftOver = 1;
                        }
                    }
                }
            }
            // Skip over the discarded sdu.
            mVTSDU++;
            sdu->free();
            continue;
        }

        sducnt++;	// We will output all or part of this sdu.
        unsigned sdusize = sdu->size();	// Size of the whole remaining SDU.

        // Note: a non-zero mLILeftOver flag here implies this is the start of a new SDU,
        // but not the reverse, ie, mLILeftOver == 0 says nothing about start of SDU.
        if (mConfig->UM.mAltEBitInterpretation) {
            // We dont use "alternative-E bit interpretation".
            // The following special LI values are used only with "alternative E-bit interpretation"
            // 0x7ffa (1010), 0x7ffd (1101), 0x7ffe (1110) as first LI field
            assert(0);
        } else {
            // Handle the special case of extra LI field to mark previous PDU.
            // The previous PDU ended on, or 1 byte short of, the end of the PDU.
            if (mLILeftOver) {
                assert(licnt == 0);
                remaining -= libytes;	// For the LI field we are about to add.
                vli[licnt++] = (mLILeftOver == 1) ? 0 : 0x7ffb;
                mLILeftOver = 0;
            }
        }

        // And now, back to our originally scheduled programming:
        // Notes: the 0x7ffc (1100) special LI value is only used in uplink, not downlink.
        // 5-15-2012: However, we are going to try putting it in anyway; the spec says
        // the UE 'should be prepared to receive it' on downlink.
        // We might want to do this only if mConfig->mIsSharedRlc
        if ((result->rlcMode() == URlcModeUm) && (licnt == 0 && newSdu) ) {
            remaining -= libytes;	// For the LI field we are about to add.
            vli[licnt++] = 0x7ffc;
        }

        if (sdusize > remaining) {
            sdufinalbytes = remaining;
            remaining = 0;
            // No LI indicator - this sdu spills over the end of the pdu into the next pdu.
            //mSduIsSplit = true;
        } else if (sdusize == remaining) {
            mLILeftOver = 1;	// Special case: LI indicator goes in next pdu.
            remaining = 0;
        } else if (libytes == 2 && sdusize == remaining-1) {
            mLILeftOver = 2;	// Special case: LI indicator goes in next pdu.
            remaining = 1;
        } else if (libytes == 2 && sdusize == remaining-3) {
            // In above, 3 == 2-byte LI + 1 more byte, which is not enough for another LI field.
            vli[licnt++] = sdusize;
            remaining = 1;
        } else if (libytes == 2 && sdusize == remaining+1) {
            // Split pdu and put the final byte in the next pdu.
            sdufinalbytes = remaining;
            vli[licnt++] = remaining;
            //mSduIsSplit = true;
            remaining = 0;
        } else {
            remaining -= libytes;
            assert(sdusize <= remaining);	// We handled all the other special cases above.
            vli[licnt++] = sdusize;
            remaining -= sdusize;
        }

        if (remaining <= libytes) {
            // Not enough room left to do anything useful.
            break;
        }

        if (mConfig->UM.mSN_Delivery) {	// This config option means do not concatenate SDUs in a PDU
            break;
        }
        if (licnt == 100) break;
        //RLCLOG("fillpdu resultsize:%d sdusize=%d sdufinalbytes=%d remaining=%d",result->size(),sdusize,sdufinalbytes,remaining);
		char buf[300] = { 0 };
		sprintf(buf, "fillpdu resultsize:%d sdusize=%d sdufinalbytes=%d remaining=%d", result->size(), sdusize, sdufinalbytes, remaining);
		PATLOG(LOG_DEBUG, buf);
    } // for sdu

    if (remaining >= libytes) {
        result->mPaddingLILocation = pduHeaderSize + licnt * libytes;
        result->mPaddingStart = result->size() - remaining;
        vli[licnt++] = 0x7fff;	// Mark padding
        remaining -= libytes;
    } else {
        result->mPaddingLILocation = 0;	// This is an impossible value.
        result->mPaddingStart = 0;
    }

    // Create the result ByteVector, add the LI+E fields.
    // E==1 implies a following LI+E field.
    // We dont implement the "Alternative E-bit interpretation".
    result->setEIndicator(licnt ? 1 : 0);	// Set the E or HE bit in the header.
    result->setAppendP(pduHeaderSize);
    if (licnt) {
        for (int i = 0; i < licnt; i++) {
            result->appendLIandE(vli[i],i != licnt-1,libytes);
        }
        RLCLOG("fillpdu after adding li, headersize:%d",result->size());
    } else {
        RLCLOG("fillpdu no li, headersize:%d",result->size());
    }

    // Step two: add the data from sducnt SDUs to the result PDU.
    for (unsigned n = 0; n < sducnt; n++) {
        if (mSplitSdu) {
            sdu = mSplitSdu; mSplitSdu = NULL;
        } else {
            sdu = mSduTxQ.pop_front();
        }

        // Need these checks to assure the mSplitSdu didn't just get discarded b/c mSduTxQ is too big.
        if (!sdu) {LOG(NOTICE) << "NULL Pointer in mSduTxQ"; break;}	// This is a bug.
        if (!sdu->sduData()->size()) { LOG(INFO) << "Empty SDU in mSduTxQ"; break;} // This is a bug.
        // If this is the final sdu and it is a partial one:

        if (n+1 == sducnt && sdufinalbytes) {
            // Copy part of this sdu.
            //LOG(INFO) << "sduData: " << *(sdu->sduData());
            result->append(sdu->sduData()->begin(),sdufinalbytes);
            //printf("sdu->sduData(): %0x\n",sdu->sduData());
            sdu->sduData()->trimLeft(sdufinalbytes);
            mSplitSdu = sdu;
            RLCLOG("fillpdu appending (partial) %d sdu bytes, result=%d bytes",
                   sdufinalbytes, result->size());
        } else {
            // Copy the entire SDU.
            result->append(sdu->sduData());
            RLCLOG("fillpdu appending %d sdu bytes, result=%d bytes",
                   sdu->sduData()->size(), result->size());
            mVTSDU++;
            sdu->free();
            //printf("Freeing SDU: %0x\n",sdu);
            /***  We now pad the entire remaining with 0, below.
            if (n+1 == sducnt && result->size() == dataSize-1) {
                // Special case of 1 extra byte:  It is filled with 0.
                // "In the case where a PDU contains a 15-bit "Length Indicator" indicating
                // that an RLC SDU ends with one octet left in the PDU,
                // the last octet of this PDU shall:
                // - be padded by the sender and ignored by the Receiver though there is no
                // "Length Indicator" indicating the existence of padding.
                result->appendByte(0);
            }
            ***/
        }
    }

    if (remaining) {
        result->appendFill(0,remaining);
    }
    mVTPDU++;
    return mSduDiscarded;
}


// MAC reads the low side with this.
// Caller is responsible for deleting this.
URlcBasePdu *URlcTransAmUm::rlcReadLowSide()
{
	URlcPdu *pdu;
	if (mRlcState == RLC_STOP) { return NULL; }
	bool wasqueued = true;
	if (!(pdu = mPduOutQ.readNoBlock())) {
		pdu = readLowSidePdu();
		wasqueued = false;
	}
	if (pdu){
		/*	RLCLOG("readlLowSide(q=%d,sizebytes=%d,payloadsize=%d,descr=%s,rb=%d header=%s)",
			(int)false,pdu->size(),pdu->getPayloadSize(),
			pdu->mDescr.c_str(),mrbid,pdu->segment(0,2).hexstr().c_str());*/

		char buf[300] = { 0 };
		sprintf(buf, "readlLowSide(q=%d,sizebytes=%d,payloadsize=%d,descr=%s,rb=%d header=%s)", (int)false, pdu->size(), pdu->getPayloadSize(),
			pdu->mDescr.c_str(), mrbid, pdu->segment(0, 2).hexstr().c_str());
		PATLOG(LOG_DEBUG, buf);
	}
	return pdu;
}

// Pull data through the RLC to fill the output queue, up to the specified amt,
// which is the maximum amount needed for any Transport Format.
void URlcTransAmUm::rlcPullLowSide(unsigned amt)
{
    URlcPdu *vec;
    int cnt = 0;
    while (mPduOutQ.totalSize() < amt && ((vec = readLowSidePdu())) ) {
        mPduOutQ.write(vec);
        cnt++;
        //LOG(INFO) << "amt: " << amt << " sz: " << mPduOutQ.totalSize();
    }

    if (cnt) LOG(INFO) << format("rlcPullLowSide rb%d amt=%d sent %d pdus, pduq=%d(%dB), sduq=%d(%dB)",
                mrbid,amt,cnt,mPduOutQ.size(),mPduOutQ.totalSize(),mSduTxQ.size(),mSduTxQ.totalSize());
}

void URlcTransAmUm::textAmUm(std::ostream &os)
{
    os <<LOGVAR2("mDlPduSizeBytes",mConfig->mDlPduSizeBytes)
        <<LOGVAR2("PduOutQ.size",mPduOutQ.size())
        <<LOGVAR(mVTPDU)<<LOGVAR(mLILeftOver)
        <<LOGVAR2("rlcGetBytesAvail",rlcGetBytesAvail())
        <<LOGVAR2("rlcGetPduCnt",rlcGetPduCnt());
}

void URlcTransAmUm::transDoReset()
{
    mLILeftOver = 0;
    mVTPDU = 0;
    mVTSDU = 0;
    ScopedLock lock(mQLock);	// We are touching mSplitSdu
    if (mSplitSdu) {
        // First sdu was partially sent; throw it away.
        mSplitSdu->free();
        mSplitSdu = NULL;
    }
}

unsigned URlcTransAmUm::rlcGetBytesAvail() {
    if (mRlcState == RLC_STOP) {return 0;}
    // Can the pdus move from one queue to the other in between the locks?
    return rlcGetSduQBytesAvail() + mPduOutQ.totalSize();
}

// Return the size of the top PDU, or 0 if none.
unsigned URlcTransAmUm::rlcGetFirstPduSizeBits() {
    if (mRlcState == RLC_STOP) {return 0;}
    ByteVector *pdu = mPduOutQ.front();
    return pdu ? pdu->sizeBits() : 0;
}
// If mLILeftOver is non-zero then we still need to send another PDU.
bool URlcTransAmUm::pdusFinished() { return getSduCnt() == 0 && !mLILeftOver; }
