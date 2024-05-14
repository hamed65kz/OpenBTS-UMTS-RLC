#include "URlcTrans.h"
#include "../../Utilities/Logger.h"


// This is already associated with a specific RAB by being
// called within this particular URlcUMT
// Clause 8.1: The parameters are: Data, CNF Confirmation Request,
// which we wont implement, DiscardReq, which we will,
// MUI Message Unit Id for this sdu in confirm/discard messages back to layer3,
// and UE-Id type indicator (C-RNTI or U-RNTI), about which I have no clue yet,
// and doesnt make sense for what I know - the C-RNTI is only used on phy, not up here.
// Update: maybe the ue-id type indicator is used when the controlling RNC != serving RNC, which we never do.
// nope: We're going to use CNF for both Confirmation and Discard requests.
// This is a temporary stub to send discard messages.
// This is what is defined in the spec, because normally these message would
// go to an MSC controller far away.  But we will probably do not do it this way,
// we will probably have a UEInfo struct and call an appropriate function
// inside the UE if messages are discared on SRBs, which indicates that the UE
// has become disconnected.
void informL3SduLoss(URlcDownSdu *sdu)
{
    PATLOG(1,"discarding sdu sizebytes="<<sdu->size());
    LOG(WARNING) << "discarding sdu sizebytes="<<sdu->size();
    // There, you're informed.
}

void URlcTrans::rlcWriteHighSide(ByteVector& data, bool DiscardReq, unsigned MUI, string descr)
{
	/*RLCLOG("rlcWriteHighSide sizebytes=%d rbid=%d descr=%s",
		data.size(), mrbid, descr.c_str());*/

	char buf[300] = { 0 };
	sprintf(buf, "rlcWriteHighSide sizebytes=%d rbid=%d descr=%s", data.size(), mrbid, descr.c_str());
	PATLOG(LOG_DEBUG, buf);

	//pat 12-17: Changed the GGSN to pre-allocate this so we dont have to do it here.
	//ByteVector cloneData;
	//cloneData.clone(data);
	//URlcDownSdu *sdu = new URlcDownSdu(cloneData,DiscardReq,MUI,descr);
	URlcDownSdu* sdu = new URlcDownSdu(data, DiscardReq, MUI, descr);

	ScopedLock lock(mQLock);
	//printf("pushing SDU of size: %u, addr: %0x, descr=%s\n",data.size(),sdu,descr.c_str());
	mSduTxQ.push_back(sdu);

	LOG(INFO) << "Bytes avail: " << rlcGetSduQBytesAvail();
	// Check for buffer overflow.
	if (!mSplitSdu && rlcGetSduQBytesAvail() > mTransmissionBufferSizeBytes)
	{
		// Throw away sdus.  We toss the oldest sdu that is not in progress,
		// which means just toss them off the front of the queue.
		URlcDownSdu* sdu = NULL;

		// Delete all but one of the sdus.
		// We leave one sdu in the queue to mark the spot where deletions ocurred.
		//LOG(INFO) << "Bytes avail: " << rlcGetSduQBytesAvail();
		while (mSduTxQ.size() && rlcGetSduQBytesAvail() > mTransmissionBufferSizeBytes) {
			//if (sdu) { sdu->free(); mVTSDU++; }
			if (sdu) { sdu->free(); mVTSDU++; }
			sdu = mSduTxQ.pop_front();
			LOG(INFO) << "Discarding sdu %0x ," << sdu << " TxQ size: " << rlcGetSduQBytesAvail();
			if (!sdu->mDiscarded) { informL3SduLoss(sdu); }
		}

		// Shove the last deleted sdu back in the queue to mark the spot.
		// We null out mData to indicate that one or more sdus were deleted at this spot.
		//if (sdu->mData) {
			//delete sdu->mData;
			//sdu->mData = NULL;
		//}
		//sdu->mDiscarded = true;
		//mSduTxQ.push_front(sdu);
	}
}

// About the LI Length Indicator field.
// If the SDU exactly fills the PDU and there is no room for an LI field,
// you set the LI in the subsequent PDU as follows:
// - If the next PDU is exactly filled by a complete SDU (remembering that the length
// of the following PDU is less 1 to make room for this extra LI field)
// then use LI=0x7d, otherwise, LI=0.
// Note that the PDU must be sent even it if has not SDU data.
// If the PDU has padding, add LI=0x7f after the final length indicator.
// The following is mandatory for uplink, and 11.2.3.1 implies that it is optional in downlink.
// Every PDU that starts a new SDU and doesnt have any of the other special LI values,
// must start with LI=0x7c, so the nodeB can tell it is the start of a packet even if it did not
// receive the previous PDU.


// It cant be const.
void URlcTrans::textTrans(std::ostream& os)
{
	ScopedLock lock(mQLock);	// We are touching mSplitSdu
	os << LOGVAR(mVTSDU);
	os << LOGVAR2("mSplitSdu.size", (mSplitSdu ? mSplitSdu->size() : 0));
	os << LOGVAR2("getSduCnt", getSduCnt());
	os << LOGVAR2("rlcGetSduQBytesAvail", rlcGetSduQBytesAvail());
	os << LOGVAR2("rlcGetPduCnt", rlcGetPduCnt());
	os << LOGVAR2("rlcGetFirstPduSizeBits", rlcGetFirstPduSizeBits());
	os << LOGVAR2("rlcGetDlPduSizeBytes", rlcGetDlPduSizeBytes());
}

URlcTrans::URlcTrans() : mSplitSdu(0), mVTSDU(0) {
    mTransmissionBufferSizeBytes = UMTS_RLC_TransmissionBufferSize;
}
unsigned URlcTrans::rlcGetSduQBytesAvail() {
	ScopedLock lock(mQLock);
	unsigned partialsize = mSplitSdu ? mSplitSdu->size() : 0;
	return mSduTxQ.totalSize() + partialsize;
}
unsigned URlcTrans::getSduCnt() {
	ScopedLock lock(mQLock);	// extra cautious
	return mSduTxQ.size() + (mSplitSdu ? 1 : 0);
}
