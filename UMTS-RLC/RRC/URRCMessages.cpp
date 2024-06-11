#include "URRCMessages.h"
#include "../Utilities/Logger.h"
#include "../configurations.h"
#include "../Utilities/Logger.h"
#include "Configs/RrcMasterChConfig.h"
#include "../Ue/UEInfo.h"
#include "../RRC/URRC.h"
#include "../RLC/Tran/URlcTransUm.h"
#include "../RLC/URlcPair.h"


unsigned macHeaderSize(TrChType tctype, ChannelTypeL3 lch, bool multiplexing);

bool AsnUeId::RaiMatches()
{
	// MCC and MNC from the PLMN_identity must exist and match.
	//if (mMcc.size() == 0 || mMnc.size() == 0) { return false; }
    const string mccstr = UMTS_Identity_MCC;
    const string mncstr = UMTS_Identity_MNC;
	unsigned mcc = atoi(mccstr.c_str());
	unsigned mnc = atoi(mncstr.c_str());
	//ByteVector mcc = stringOfDigitsToByteVector(mccstr);
	//ByteVector mnc = stringOfDigitsToByteVector(mncstr);
	if (mMcc != mcc && mMnc != mnc) {
		LOG(INFO) << "RAI of UE"<<LOGVAR(mcc)<<LOGVAR(mnc) <<" does not match"<<LOGVAR(mMcc)<<LOGVAR(mMnc);
		return false;
	}

	// LAC and RAC must match.
    unsigned lac = UMTS_Identity_LAC;
    unsigned rac = GPRS_RAC;
	if (lac != mLac || rac != mRac) {
		LOG(INFO) << "RAI of UE" <<LOGVAR(lac)<<LOGVAR(rac)<<" does not match"<<LOGVAR(mLac)<<LOGVAR(mRac);
		return false;
	}
	LOG(INFO) << "RAI of UE matches";
	return true;
}
bool AsnUeId::eql(AsnUeId &other)
{
	// All fields are inited, so just compare everything.
	if (idType != other.idType) return false;
	if (mImsi != other.mImsi || mImei != other.mImei || mTmsiDS41 != other.mTmsiDS41) return false;
	if (mMcc != other.mMcc || mMnc != other.mMnc) return false;
	if (mTmsi != other.mTmsi || mPtmsi != other.mPtmsi || mEsn != other.mEsn) return false;
	if (mLac != other.mLac || mRac != other.mRac) return false;  
	return true;
}


URlcTransUm **mCcchRlc=NULL;// check invalid index
void writeHighSideCcch(ByteVector &sdu, const string descr, int nodeBIndex)
{
	// Run the message through the RLC-UM entity for CCCH.
		PATLOG(1, "MaccBase::writeHighSideCcch sizebits:" << sdu.sizeBits() << " " << descr);
		mCcchRlc[nodeBIndex]->rlcWriteHighSide(sdu, 0, 0, descr);
}

ByteVector * macReadFromCCCH(int nodeBId){
	ByteVector *pdu = mCcchRlc[nodeBId]->rlcReadLowSide();

	return pdu;
}
void setupDlRlcCCCH(int nodeBCount){
	// Set up the CCCH downlink rlc entity.  Needs a dummy RBInfo for SRB0, but the SRB0
	// info is not dummied - it comes from the RRC spec sec 13.6.
	// The SRB0 
	RBInfo rbtmp;
	rbtmp.defaultConfigSRB0();

	int dlmacbits = macHeaderSize(TrChFACHType, CCCHType, true);
	assert(dlmacbits == 8); // We could have just said: 8.

	mCcchRlc = new URlcTransUm * [nodeBCount];
	unsigned trbksize = mTBSz;
	for (int i = 0; i < nodeBCount; i++)
	{
		mCcchRlc[i] = new URlcTransUm(&rbtmp,
			0,      // no TFS required, and in fact, this param is no longer used and should be elided.
			0,      // no associated UE.
			(trbksize - dlmacbits) / 8,
			true);  // This is the shared RLC for Ccch.
	}
}

