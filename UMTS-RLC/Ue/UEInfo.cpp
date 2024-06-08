#include "UEInfo.h"
#include "../Utilities/Utils.h"
#include "../RLC/URlcPair.h"
#include "../RRC/URRC.h"

Rrc gRrc;

#define RN_UE_FOR_ALL_RLC_DOWN(uep,rbid,rlcp) \
	URlcTrans *rlcp; \
for (RbId rbid = 0; rbid <= uep->mUeMaxRlc; rbid++) \
if ((rlcp = uep->getRlcDown(rbid)))

#define CASENAME(x) case x: return #x;

const char* UEState2Name(UEState state)
{
    switch (state) {
        CASENAME(stUnused)
            CASENAME(stIdleMode)
            CASENAME(stCELL_FACH)
            CASENAME(stCELL_DCH)
            CASENAME(stCELL_PCH)
            CASENAME(stURA_PCH)
            // Do not add a default case here; we want the error message if this
            // function gets out of phase with the enumeration.
    }
    return "unrecognized";
};

string UEInfo::ueid() const
{
    char buf[100];
    sprintf(buf," UE#%d URNTI=%x",mUeDebugId,(uint32_t)mURNTI);
    return string(buf); // not efficient, but only for debugging.
}

UeTransaction::UeTransaction(UEInfo* uep, TransType ttype, RbId wRabMask, unsigned wTransactionId, UEState newState)
{
    //mTransactionId = uep->newTransactionId();
    mTransactionId = wTransactionId;
    mNewState = newState;
    mRabMask = wRabMask;
    mTransactionType = ttype;
    mTransTime.now();
    // Squirrel it away in the UE.
    //uep->mTransactions[mTransactionId] = *this;
}

std::string UeTransaction::str()
{
    return format("UeTransaction(type=%d=%s newState=%s rabMask=%d transId=%d)",
        mTransactionType, TransType2Name(mTransactionType),
        UEState2Name(mNewState), mRabMask, mTransactionId);
}

long UeTransaction::elapsed()	// How long since the transaction, in ms?
{
    return mTransTime.elapsed();
}

//UeTransaction *UEInfo::getLastTransaction()
//{
//	if (mNextTransactionId == 0) return NULL;	// No transactions have occurred yet.
//	return &mTransactions[(mNextTransactionId - 1) % UEDefs::sMaxTransaction];
//}

// This receives messages on DCCH on either RACH or DCH.
// It does not receive message on CCCH.
// The UEState here specifies whether the message arrived on RACH (state==stCell_FACH)
// or DCH (state==stCELL_DCH), and it is an error it does not match the state we think the UE is in.
void UEInfo::ueWriteLowSide(RbId rbid, const BitVector &pdu, UEState state)
{
	ueRegisterActivity();
	//UeTransaction(uep,UeTransaction::ttRrcConnectionSetup, 0, transactionId,stCELL_FACH);
	// The first transaction needs to be handled specially.
	// The first message, RrcConnectionSetupComplete, must be handled specially.
	// The fact of its arrival is confirmation of the transaction.
	if (mUeState == stIdleMode) {
		// It is NOT necessarily transaction number 0.
		// FIXME: This may have missed the uplink ConnectionSetupComplete message, in which case what should we do?
		/*UeTransaction *trans = getLastTransaction();
		if (trans && trans->mTransactionType == UeTransaction::ttRrcConnectionSetup) {
			ueRecvRrcConnectionSetupResponse(0, true, "RrcConnectionSetup");
		}
		else {
			LOG(ERR) << "Received DCCH message for UE in idle mode for trans. type: " << trans->mTransactionType << " " << this;
		}*/
	}
	URlcRecv *rlc = NULL;
	if (rbid <= mUeMaxRlc) {
		rlc = getRlcUp(rbid, state);
	}
	if (!rlc) {
		LOG(ERR) << "invalid" << LOGVAR(rbid) << ((state == stCELL_FACH) ? " on RACH" : " on DCH") << this;
		return;
	}
	LOG(INFO) << "rbid: " << rbid << " rrc: rlcWriteLowSide: " << this << " " << pdu;
	rlc->rlcWriteLowSide(pdu);
	// TODO: This rlc needs a connection on the top side.
}
void UEInfo::ueRegisterActivity()
{
	mActivityTime.now();
}
std::ostream& operator<<(std::ostream& os, const UEInfo*uep)
{
	if (uep) {
		os << uep->ueid();
	}
	else {
		os << " UE#(null)";
	}
	return os;
}

URlcPair *UEInfo::getRlc(RbId rbid)
{
	switch (mUeState) {
	case stCELL_FACH:
	case stIdleMode: // FIXME:: This happens from time to time.  patching over for now.
		// pat 12-30-2012:  We were putting the UE in idle mode accidentally,
		// so maybe this is fixed now, but I am leaving this case here.
		//LOG(INFO)<<format("getRlc this=%p URNTI=%x rbid=%d, 1=%p 2=%p 3=%p 2up=%p\n",this,
		//	mURNTI,rbid,mRlcsCF[1],mRlcsCF[2],mRlcsCF[3],mRlcsCF[2]?mRlcsCF[2]->mUp:0);
		return mRlcsCF[rbid];
	case stCELL_DCH:
		return mRlcsCDch[rbid];
	default:
		assert(0);
	}
}

URlcTrans *UEInfo::getRlcDown(RbId rbid)
{
	URlcPair *pair = getRlc(rbid);
	return pair ? pair->mDown : 0;
}

URlcRecv *UEInfo::getRlcUp(RbId rbid, UEState state)
{
	// pat 12-29-2012: Setting uestate here is very wrong, because we can get garbage coming
	// in the from the radio at any time and we dont want to misconfigure the RLC here.
	// mUeState = state;
	URlcPair *pair = getRlc(rbid);
	return pair ? pair->mUp : 0;
}

void UEInfo::ueRecvRrcConnectionSetupResponse(unsigned transId, bool success, const char *msgname)
{
	//UeTransaction *tr = getTransaction(transId, UEDefs::TransType::ttRrcConnectionSetup, msgname);
	// Even if we dont find the correct UeTransaction, the UE is in
	// CELL_FACH state or we would not have gotten this message, so switch:
	//if (success) this->ueSetState(stCELL_FACH);
	//if (tr) tr->transClose();	// Done with this one.
}
//UeTransaction *UEInfo::getTransaction(unsigned transId, UEDefs::TransType ttype, const char *help)
//{
//	if (transId >= 4) {
//		PATLOG(1, format("%s: invalid transaction id, out of range, type:%d transId:%d",
//			help, ttype, transId));
//		return NULL;
//	}
//	UeTransaction *tr = &mTransactions[transId];
//	if (tr->mTransactionType != ttype) {
//		PATLOG(1, format("%s: invalid transaction, expected type:%d got type:%d id:%d",
//			help, ttype, tr->mTransactionType, transId));
//		return NULL;
//	}
//	return tr;
//}

// 25.331 10.3.3.36 The transaction id in the messages is only 2 bits.
unsigned UEInfo::newTransactionId() { return (mNextTransactionId++) % UEDefs::sMaxTransaction; }

//void UEInfo::ueSetState(UEState newState)
//{
//	printf("newState: %d %d\n", newState, mUeState);
//	if (newState == mUeState) return;
//	// TODO: When moving to CELL_FACH we have to set up the RBs and RLCs.
//	// Assumes only one active DCH per UE
//	if (/*mUeState == stCELL_DCH &&*/ newState != stCELL_DCH) {
//		usleep(2000); // sleep to make sure other messages get processed through the RLC, MAC, L1, etc.
//		//macUnHookupDch(this);//HKZ
//	}
//	switch (newState) {
//	case stIdleMode:
//		ueDisconnectRlc(stCELL_FACH);
//		ueDisconnectRlc(stCELL_DCH);
//		//this->integrity.integrityStop();//HKZ
//		break;
//	default: break;
//	}
//	mUeState = newState;
//}

// Destroy the RLC entities
// Take care because mRlcsCF[i] and mRlcsCDch[i] may point to the same RlcPair.
void UEInfo::ueDisconnectRlc(UEState state)
{
	int cnt = 0, deleted = 0;
	for (unsigned i = 0; i < gsMaxRB; i++) {
		switch (state) {
		case stCELL_FACH:
			if (mRlcsCF[i]) {
				if (mRlcsCF[i] != mRlcsCDch[i]) { 
					delete 	mRlcsCF[i]; 
					deleted++; 
				}
				mRlcsCF[i] = 0;
				cnt++;
			}
			break;
		case stCELL_DCH:
			if (mRlcsCDch[i]) {
				if (mRlcsCF[i] != mRlcsCDch[i]) { 
					delete mRlcsCDch[i];
					deleted++; 
				}
				mRlcsCDch[i] = 0;
				cnt++;
			}
			break;
		default: assert(0);
		}
	}
	if (cnt) LOG(INFO) << format("ueDisconnectRlc: new state=%s disconnected %d deleted %d %s",
		UEState2Name(state), cnt, deleted, this->ueid().c_str());
}

// Connect this UE to some RLCs for the RBs defined in the config for the specified new state.
// The configuration is pending until we receive an answering message with
// the specified transactionId, at which time the specified RBs will become usable.
// NOTE!!  If the rlc-mode is the same (ie, TM,UM,AM) and the rlc-size does
// not change, the spec requires us to 'copy' the internal state when
// we change state, which we affect by simply re-using the same RLC,
// which implies we are not allowed to change the RLC programming during
// the state change.
void UEInfo::ueConnectRlc(
	RrcMasterChConfig *config, 	// This is the config we will use in nextState.
	UEState nextState)			// The state we are trying to configure.
{
	// Even if we were in DCH state before, we must look for up any new RBs that need RLCs.
	//if (config == mUeConfig) {return;}	// WRONG!
	bool isNewState = false;//(nextState != ueGetState());
	// Note that when we move to CELL_FACH state we may be leaving RLCs from DCH state on rb 5-15.
	for (unsigned i = 1; i < config->mNumRB; i++) {	// Skip SRB0
		RBInfo *rb = config->getRB(i);
		if (rb->valid()) {
			unsigned rbid = rb->mRbId;
			// You may ask: we are allocating both up and down RLCs,
			// but we are only sending the downlink TFS, and the uplink may be different.
			// It is because we need the downlink TFS to compute some parameter
			// for the downlink RLC that is specified directly as a parameter for uplink RLC.
			// Update: now all we use dltfs for is to compute the mac header size,
			// so we should probably just send that instead.
			TrChId tcid = rb->mTrChAssigned;
			RrcTfs *dltfs = config->getDlTfs(tcid);
			if (rbid > mUeMaxRlc) { mUeMaxRlc = rbid; }

			// TODO: This may want to hook a PDCP on top of the RLC.
			URlcPair *other, *prev;
			unsigned newpdusize = computeDlRlcSize(rb, dltfs);
			const char *action = "";
			switch (nextState) {
			case stCELL_FACH:
				// First delete existing if necessary.
				prev = mRlcsCF[rbid];
				if (!isNewState && prev) { continue; }	// Leave previously configured RBs alone.
				other = mRlcsCDch[rbid];
				if (prev && prev != other) { 
					LOG(INFO) << "Delete previous rlc urnti : " << this->mURNTI << " crnti: " << this->mCRNTI << " rbid " << rbid;
					delete prev;
				}
				if (other && other->mDown->mRlcMode == rb->getDlRlcMode() && other->mDown->rlcGetDlPduSizeBytes() == newpdusize) {
					mRlcsCF[rbid] = other;
					action = "copied";
				}
				else {
					mRlcsCF[rbid] = new URlcPair(rb, dltfs, this, tcid);
					action = "allocated";
				}
				//if (isNewState) this->mStateChange = true;
				break;
			case stCELL_DCH:
				prev = mRlcsCDch[rbid];
				if (!isNewState && prev) { continue; }	// Leave previously configured RBs alone.
				other = mRlcsCF[rbid];
				if (prev && prev != other) {
					LOG(INFO) << "Delete previous rlc urnti : "<<this->mURNTI<<" crnti: "<< this->mCRNTI << " rbid " << rbid;
					delete prev; 
				}
				if (other) printf("PDU sizes: %d %d\n", other->mDown->rlcGetDlPduSizeBytes(), newpdusize);
				if (other && other->mDown->mRlcMode == rb->getDlRlcMode() && other->mDown->rlcGetDlPduSizeBytes() == newpdusize) {
					mRlcsCDch[rbid] = other;
					action = "copied";
				}
				else {
					mRlcsCDch[rbid] = new URlcPair(rb, dltfs, this, tcid);
					action = "allocated";
				}
				//if (isNewState) this->mStateChange = true;
				//mRlcsCDch[rbid]->mDown->triggerReset();
				break;
			default: assert(0);
			}

			LOG(INFO) << format("ueConnectRlc: next=%s %s i=%d rbid=%d up=%s down=%s %s",
				 UEState2Name(nextState), action,
				i, rbid, URlcMode2Name(rb->getUlRlcMode()), URlcMode2Name(rb->getDlRlcMode()), this->ueid().c_str());
		}
	}
	//LOG(INFO)<<format("connectRlc this=%p URNTI=%x 1=%p 2=%p 3=%p 2up=%p\n",this,
	//	mURNTI,mRlcsCF[1],mRlcsCF[2],mRlcsCF[3],mRlcsCF[2]?mRlcsCF[2]->mUp:0);
	//mUeConfig = config;
}

void UEInfo::ueWriteHighSide(RbId rbid, ByteVector &sdu, string descr)
{
	ueRegisterActivity();
	PATLOG(1, format("ueWriteHighSide(%d,sizebytes=%d,%s)", rbid, sdu.size(), descr.c_str()));
	LOG(INFO) << "From SGSN: " << format("ueWriteHighSide(%d,sizebytes=%d,%s)", rbid, sdu.size(), descr.c_str());
	LOG(INFO) << "SGSN data: " << sdu;
	URlcTrans *rlc = getRlcDown(rbid);
	if (!rlc) {
		LOG(ERR) << "logic error in ueWriteHighSide: null rlc";
		//delete sdu;
		return;
	}
	rlc->rlcWriteHighSide(sdu, 0, 0, descr);
}


UEInfo::UEInfo(AsnUeId *wUid) : mUid(*wUid)
{
    _initUEInfo();
    // Allocate a RNTI for this new UE.
    gRrc.newRNTI(&mURNTI,&mCRNTI);
    //connectUeRlc(gRrcCcchConfig);	// Not necessary
    gRrc.addUE(this);
}
UEInfo::UEInfo(uint32_t urnti, uint16_t crnti) {
	// OK to leave AsnUeId empty.
	_initUEInfo();
	mURNTI = urnti;
    mCRNTI = crnti;
	gRrc.addUE(this);
}
// Stupid language.
void UEInfo::_initUEInfo() {
				
				 mUeState = stIdleMode;
				 //mUeDch = NULL;
				 //mUeMac = NULL;
				 //mGsmL3 = allocateLogicalChannel();
				 memset(mRlcsCF, 0, sizeof(mRlcsCF));
				 memset(mRlcsCDch, 0, sizeof(mRlcsCDch));
				 mHelloTime.now();
				 mActivityTime = mHelloTime;
				 mURNTI = 0;	// We expect these to be set immediately, but cant be too cautious.
				 mCRNTI = 0;
				 //radioCapability = NULL;
	}

void UEInfo::uePullLowSide(unsigned amt)
{
	RN_UE_FOR_ALL_RLC_DOWN(this, rbid, rlcp) {
		rlcp->rlcPullLowSide(amt);
	}
}

void UEInfo::ueRecvDcchMessage(ByteVector &bv, unsigned rbNum){
	ueRegisterActivity();	// Not sure if all these messages count as activity.
	UEInfo *uep = this;
	//bool verbose = gConfig.getNum("UMTS.Debug.Messages"); // redundant with asnLogMsg.
	//if (verbose) { fprintf(stdout,"Received DCCH message for %s rb=%d\n",uep->ueid().c_str(),rbNum); }
	LOG(INFO) << "DCCH ByteVector: " << bv;

	UL_DCCH_MessageType_PR MsgType = UL_DCCH_MessageType_PR::UL_DCCH_MessageType_PR_activeSetUpdateComplete;
	if (MsgType == UL_DCCH_MessageType_PR_uplinkDirectTransfer)
	{
		ByteVector l3UdtMsgBytes;
		uep->ueRecvL3Msg(l3UdtMsgBytes, uep);
	}
	else if (MsgType == UL_DCCH_MessageType_PR_initialDirectTransfer)
	{
		ByteVector l3IdtMsgBytes;
		uep->ueRecvL3Msg(l3IdtMsgBytes, uep);
	}
}
void UEInfo::ueRecvL3Msg(ByteVector &msgframe, UEInfo *uep)
{
	unsigned pd = msgframe.getNibble(0, 0);	// protocol descriminator
	LOG(INFO) << "Received L3 message of with protocol discriminator " << pd;
	switch ((L3PD) pd) {
	case L3PD::L3GPRSMobilityManagementPD:	// Couldnt we shorten this?
	case L3PD::L3GPRSSessionManagementPD: 	// Couldnt we shorten this?
	{
												//LOG(INFO) << "Sending L3 message of descr " << pd << "up to SGSN"; 
												//sgsnHandleL3Msg(uep->mURNTI, msgframe);
												ByteVector dlpdu(1000); //Fill by MAli
												string desc = "";
												ueWriteHighSide(SRB3, dlpdu, desc);
												//LOG(INFO) << "Sent to SGSN";
												break;
												// TODO: Send GSM messages somewhere
	}
	case L3PD::L3CallControlPD:
	case L3PD::L3MobilityManagementPD:
	case L3PD::L3RadioResourcePD:
	{
									// In GSM these go on the logical channel, which is polled by DCCHDispatcher,
									// then calls DCCHDispatchMessage, which then calls some sub-processor
									// which may generate message traffic on a LogicalChannel class.
									// The best way to interface to the existing code is probably to put
									// these on the LogicalChannel and let the DCCH service loop find them.
									// It wants to find an ESTABLISH primitive as the first thing,
									// and then it times out if nothing happens soon?
									// Or we could try calling direct: DCCHDispatchMessage(??,??);

									// FIXME: Ignore these until L3 GSM code is integrated
									LOG(ERR) << "L3 GSM control message ignored";
									//uep->mGsmL3->l3writeHighSide(msgframe);
									return;
	}
	case L3PD::L3SMSPD:
	{
						  // In GSM these apparently arrive on the DTCHLogicalChannel?
						  // Not sure how SMS works.  Looks like an MM message CMServiceRequest
						  // arrives to DCCHDispatchMM() which then calls CMServiceResponder()
						  // which calls MOSMSController() which seems to do a bunch of message
						  // traffic on a DCCHLogicalChannel, and SMS messages go there.
						  LOG(ERR) << "L3 SMS Message ignored";
						  return;
	}
	default:
		LOG(ERR) << "unsupported L3 Message PD:" << pd;
	}
}
