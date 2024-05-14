#include "URRCMessages.h"
#include "../Utilities/Logger.h"
#include "../configurations.h"
#include "../Utilities/Logger.h"
#include "Configs\RrcMasterChConfig.h"
#include "../Ue/UEInfo.h"
#include "../RRC/URRC.h"
#include "../RLC/Tran/URlcTransUm.h"
#include "../RLC/URlcPair.h"

void writeHighSideCcch(ByteVector &sdu, const string descr);
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

// This message is passed transparently from the MAC using RLC-TM.
// The MAC should pop off the headers and pass the rest.
// The only possible messages are:
//		RRC Connection Request
//		Cell Update
//		URA Update
void rrcRecvCcchMessage(BitVector &tb, unsigned asc)
{
	bool handleCellUpdate = true;
	if (handleCellUpdate){
		uint32_t urnti = 0;//fill by MAli
		UEInfo *uep = gRrc.findUeByUrnti(urnti);
		if (uep) {
			//switch (uep->ueGetState()) {
			switch (stCELL_FACH) {
			case stCELL_DCH:
			{
							   // The CellUpdateConfirm message may be sent out on either DCCH or CCCH.
							   // This version is for DCCH.
							   // TODO: It would be wise to implement the RLC re-establish indicators.
							   //sendCellUpdateConfirmDcch
							   ByteVector result(1000);//fill by MAli

							   std::string descrCellUpdateConfirm = "";//fill by MAli
							   uep->ueWriteHighSide(SRB2, result, descrCellUpdateConfirm);
							   break;
			}
			default:
			{
					   //sendCellUpdateConfirmCcch
					   ByteVector result(1000);//fill by MAli

					   std::string descrCellUpdateConfirm = "";//fill by MAli
					   writeHighSideCcch(result, descrCellUpdateConfirm);
					   break;
			}
			}
		}
		else{
			// This is the first we have heard from this UE.  This is illegal.
			// Sent when an unrecognized UE tries to talk to us.
			// Tell it to release the connection and start over.
			//sendRrcConnectionReleaseCcch
			ByteVector result(1000);//fill by MAli
			std::string descrRrcConnectionRelease = "";//fill by MAli
			writeHighSideCcch(result, descrRrcConnectionRelease);
		}
	}
		bool onRrcConnSetup = true;
		if (onRrcConnSetup){
			//InitialUE_Identity_t	 initialUE_Identity;
			//AsnUeId aid(msg->initialUE_Identity);
			AsnUeId aid;//fill by MAli
			const char *comment = "UL_CCCH_MessageType_PR_rrcConnectionRequest";
			UEInfo *uep = gRrc.findUeByAsnId(&aid);
			if (uep == NULL) {
				uep = new UEInfo(&aid);
				comment = "UL_CCCH_MessageType_PR_rrcConnectionRequest (new UE)";
			}

			//uep->ueSetState(stIdleMode);
			// We need to do stop integrity protection before sending the L3 Authentication message,
			// which gets wrapped in an RRC direct transfer, which must not be integrity protected.
			//uep->integrity.integrityStop();	// Redudant with code in ueSetState, but make sure.//HKZ
			// Configure the UE to be ready for incoming on the new SRBs...
			uep->ueConnectRlc(gRrcDcchConfig, stCELL_FACH);
			// Set a UeTransaction: see ueWriteLowSide for its use.
			// there is no chance that other unrelated messages can happen simultaneously.
			// But we will print an error if the received transaction does not match.
			// The Rab Mask parameter is not relevant for this transaction type.
			//UeTransaction(uep, UeTransaction::ttRrcConnectionSetup, 0, transactionId, stCELL_FACH);

			ByteVector result(1000);//fill by MAli
			std::string descrRrcConnectionSetup = "";//fill by MAli
			writeHighSideCcch(result, descrRrcConnectionSetup);
		}
}
URlcTransUm *mCcchRlc=NULL;
void writeHighSideCcch(ByteVector &sdu, const string descr)
{
	// Run the message through the RLC-UM entity for CCCH.
		PATLOG(1, "MaccBase::writeHighSideCcch sizebits:" << sdu.sizeBits() << " " << descr);
		mCcchRlc->rlcWriteHighSide(sdu, 0, 0, descr);
}

ByteVector * macReadFromCCCH(){
	ByteVector *pdu = mCcchRlc->rlcReadLowSide();
	return pdu;
}
void setupDlRlcCCCH(){
	// Set up the CCCH downlink rlc entity.  Needs a dummy RBInfo for SRB0, but the SRB0
	// info is not dummied - it comes from the RRC spec sec 13.6.
	// The SRB0 
	RBInfo rbtmp;
	rbtmp.defaultConfigSRB0();

	int dlmacbits = macHeaderSize(TrChFACHType, CCCHType, true);
	assert(dlmacbits == 8); // We could have just said: 8.

	unsigned trbksize = mTBSz;
	mCcchRlc = new URlcTransUm(&rbtmp,
		0,      // no TFS required, and in fact, this param is no longer used and should be elided.
		0,      // no associated UE.
		(trbksize - dlmacbits) / 8,
		true);  // This is the shared RLC for Ccch.
}

// This puts the phone in idle mode.
void sendRrcConnectionRelease(UEInfo *uep) //, ASN::InitialUE_Identity *ueInitialId
{
	ByteVector result(1000);//fill by MAli

	std::string descrRrcConnectionRelease = "";//fill by MAli
	unsigned transactionId = uep->newTransactionId();
	UeTransaction(uep, UeTransaction::ttRrcConnectionRelease, 0, transactionId, stIdleMode);
	uep->ueWriteHighSide(SRB2, result, descrRrcConnectionRelease);
}
// Release 3 version of this message.  The samsung and other phones did not seem to like this,
// so 11-16-2012 tried switching to release 4 version.
void sendRrcConnectionSetup(UEInfo *uep){

	//LOG(INFO) << "gNodeB: " << gNodeB.clock().get() << ", SCCPCH: " << result;

	// TODO: This crashes
	// ASN_STRUCT_FREE_CONTENTS_ONLY(ASN::asn_DEF_DL_CCCH_Message,&msg);

	// Configure the UE to be ready for incoming on the new SRBs...
	uep->ueConnectRlc(gRrcDcchConfig, stCELL_FACH);
	// Set a UeTransaction: see ueWriteLowSide for its use.
	// there is no chance that other unrelated messages can happen simultaneously.
	// But we will print an error if the received transaction does not match.
	// The Rab Mask parameter is not relevant for this transaction type.
	ByteVector result(1000);//fill by MAli

	std::string descrRrcConnectionSetup = "";//fill by MAli
	unsigned transactionId = uep->newTransactionId();
	UeTransaction(uep, UeTransaction::ttRrcConnectionSetup, 0, transactionId, stCELL_FACH);
	writeHighSideCcch(result, descrRrcConnectionSetup);
}

// Sent when an unrecognized UE tries to talk to us.
// Tell it to release the connection and start over.
static void sendRrcConnectionReleaseCcch(int32_t urnti)
{

	ByteVector result(1000);//fill by MAli
	std::string descrRrcConnectionRelease = "";//fill by MAli
	writeHighSideCcch(result, descrRrcConnectionRelease);
}


static void sendCellUpdateConfirmCcch(UEInfo *uep)
{
	ByteVector result(1000);//fill by MAli
	std::string descrCellUpdateConfirm = "";//fill by MAli
	unsigned transactionId = uep->newTransactionId();
	UeTransaction(uep, UeTransaction::ttCellUpdateConfirm, 0, transactionId);

	writeHighSideCcch(result, descrCellUpdateConfirm);
}