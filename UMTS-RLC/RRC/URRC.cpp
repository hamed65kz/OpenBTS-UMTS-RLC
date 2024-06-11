#include "URRC.h"
#include "../Utilities/Logger.h"
#include "Configs/RrcDefs.h"
//#include "../configurations.h"

// Throw away UEs that died.
// TODO: This needs work.  The UE will go to CELL_FACH mode just to send
// a cell_update message, but we dont want to count that as activity.

#define CASENAME(x) case x: return #x;

//Rrc gRrc;


// These are the configs for CCCH and DCCH.
// The message may be on the same FACH, distinguished by MAC header.
RrcMasterChConfig gRrcCcchConfig_s;
RrcMasterChConfig gRrcDcchConfig_s;
//RrcMasterChConfig gRrcDchPSConfig_s;
// These point to the above.  We use a pointer so they can be referred to
// elsewhere without including URRC.h first.
RrcMasterChConfig *gRrcCcchConfig = 0; 	// RB[0] is SRB0
RrcMasterChConfig *gRrcDcchConfig = 0;	// adds RB[1] through RB[3].  Still on RACH/FACH
//RrcMasterChConfig *gRrcDchPSConfig = 0;


// This macro allows deletion of the current var from the list being iterated,
// because itr is advanced to the next position at the beginning of the loop,
// and list iterators are defined as keeping their position even if elements are deleted.


// Do we need a control channel for RRC messages?
// Pick 64 SF.
// Phone required FACH separate from PCH in SIB5?
// 25.211 Table 11, pick slot formats with TFCI and larger pilot bits,
// 		probably 3, 11, 14, 15, 16.
// Table 11 for CCPCH: ditto channels.
// TODO: Init the DCH as well.
void rrcInitCommonCh(unsigned rachSF, unsigned fachSF)	// (RACHFEC *rach, FACHFEC *fach)
{
	static bool inited = 0;
	gRrc.rrcDoInit();
	if (inited) return;
	inited = 1;

	// Pat set default FACH SF to 64:
	//unsigned fachSF = gConfig.getNum("UMTS.SCCPCH.SF");
	//unsigned rachSF = gConfig.getNum("UMTS.PRACH.SF");

	// Currently we use a single FACH and RACH broadcast in the beacon SIB5.
	// Therefore it is used for both CCCH (SRB0) and DCCH (SRB1,2,3)
	// (The alternative is to specify a second set of RACH/FACH for DCCH in SIB6.)
	// However, the CCCH channel is handled directly by the MAC,
	// while the DCCH logical channels go through the UE struct,
	// so we use two different master configs
	// NOTE: THe CCCH config is broadcast in SIB5.  Dont screw it up.
	gRrcCcchConfig = &gRrcCcchConfig_s;
	gRrcDcchConfig = &gRrcDcchConfig_s;
	//gRrcDchPSConfig = &gRrcDchPSConfig_s;

	// Configure CCCH and SRB0 for rach/fach
	gRrcCcchConfig->mTrCh.configRachTrCh(rachSF, TTI10ms, 16, 260); //-56+8);
	// Pat modified FACH parameters to avoid rate-matching.
	//gRrcCcchConfig->mTrCh.configFachTrCh(fachSF,TTI10ms,16);
	gRrcCcchConfig->mTrCh.configFachTrCh(fachSF, TTI10ms, 12, 360); //-56+8);

	// There are no RBs for CCCH.  The SRB0 RLC is in the Macc, not the UE.
	//gRrcCcchConfig->setSRB(0)->defaultConfigSRB0();

	// Configure DCCH and SRB1, SRB2, SRB3 for rach/fach
	// Currently use same rach fach channel as CCCH.
	gRrcDcchConfig->mTrCh.configRachTrCh(rachSF, TTI10ms, 16, 260); //-56+8);
	gRrcDcchConfig->mTrCh.configFachTrCh(fachSF, TTI10ms, 12, 360); //-56+8);
	gRrcDcchConfig->setSRB(1)->defaultConfig0CFRb(1);
	gRrcDcchConfig->setSRB(2)->defaultConfig0CFRb(2);
	gRrcDcchConfig->setSRB(3)->defaultConfig0CFRb(3);
	{
		std::ostringstream RachInform, FachInform;
		RachInform << "RACH TFS:";
		gRrcCcchConfig->getUlTfs()->text(RachInform);
		FachInform << "FACH TFS:";
		gRrcCcchConfig->getDlTfs()->text(FachInform);
		LOG(INFO) << RachInform.str();
		std::cout << RachInform.str() << std::endl;
		LOG(INFO) << FachInform.str();
		std::cout << FachInform.str() << std::endl;

		//std::cout << "RACH TFS:";
		//gRrcCcchConfig->getUlTfs()->text(std::cout);
		//std::cout << "\nFACH TFS:";
		//gRrcCcchConfig->getDlTfs()->text(std::cout);
		//std::cout <<"\n";
	}
}

void Rrc::purgeUEs()
{
    time_t now; time(&now);
    ScopedLock lock(mUEListLock);
    // NOTE: The timers are described in 13.1 and default values are in 10.3.3.43 and 10.3.3.44
    // T300 is the timer for the RRC connection setup.
    // It also interacts with T302, the cell update retry timer, but we dont use those yet.
    //int t300 = gConfig.getNum("UMTS.Timers.T300",1000);
    int tInactivity = 1000*UMTS_Timers_Inactivity_Release;
    int tDelete = 1000*UMTS_Timers_Inactivity_Delete;
    //RN_FOR_ITR(UEList_t,mUEList,itr) //hkz
	
	//if ((mUEList).size()){
	//	for (
	//		UEList_t::iterator itr, next = (mUEList).begin();
	//		(itr = next++) != (mUEList).end();
	//	itr = next
	//		)
	UEList_t::iterator pre_itr = (mUEList).end();
	if ((mUEList).size()){
		for (auto itr = (mUEList).begin(); itr != (mUEList).end(); itr++)
		{

			UEInfo *uep = *itr;

			// If the UE does not respond to an RRC message, do something, but what?.
			// They are in RLC-AM mode, so no point in resending.
			//UeTransaction *last = uep->getLastTransaction();
			/****
			if (last && last->mTransactionType != ttComplete &&
			last->mTransTime.elapsed() > t300) {
			switch (last->mTransactionType) {
			case ttRrcConnectionSetup:
			// The UE will retry in 1 second, so just ignore it.
			break;
			case ttRrcConnectionRelease:
			sendRrcConnectionRelease(UEInfo *uep);
			last->transClose();
			case ttRadioBearerSetup:
			case ttRadioBearerRelease:
			default: assert(0);
			}
			LOG(DEBUG) <<"Deleting "<<uep<<" timeout after event:"<<last->name();
			mUEList.erase(itr);
			delete uep;
			}
			***/

			// No pending transactions...
			// If the UE is inactive, attempt to drop it back to idle mode or delete it

			long elapsed = uep->mActivityTime.elapsed();
			if (elapsed > tDelete)
			{
				LOG(ALERT) << "Deleting " << uep;
				mUEList.erase(itr);
				uep->ueDisconnectRlc(UEState::stCELL_FACH);
				delete uep;
				if (pre_itr != (mUEList).end())
				{
					itr = pre_itr;
				}
				else{
					itr = (mUEList).begin();
				}
				if ((mUEList).size() == 0)
				{
					break;
				}
			}


			//switch (uep->ueGetState()) {
			//case stIdleMode:
			//	if (elapsed > tDelete) {
			//		// Temporarily add an alert for this:
			//		LOG(ALERT) << "Deleting " << uep;
			//		mUEList.erase(itr);
			//		delete uep;
			//		if (pre_itr != (mUEList).end())
			//		{
			//			itr = pre_itr;
			//		}
			//		else{
			//			itr = (mUEList).begin();
			//		}
			//		
			//		continue;
			//	}
			//	break;
			//case stCELL_FACH:
			//case stCELL_DCH:
			//case stCELL_PCH:
			//	if (elapsed > tInactivity 
			//		//&&
			//		//last->mTransactionType != ttRrcConnectionRelease
			//		) {
			//		//sendRrcConnectionRelease(uep);
			//	}
			//	break;
			//default: assert(0);
			//}
			//pre_itr = itr;
		}
	}
}

void Rrc::addUE(UEInfo *ue)
{
    purgeUEs(); // Now is a fine time to purge the UE list of any dead UEs.

    ScopedLock lock(mUEListLock);
    mUEList.push_back(ue);
}

// If ueidtype is 0, look for URNTI, else CRNTI
UEInfo *Rrc::findUe(bool ueidtypeCRNTI, unsigned uehandle, uint16_t mNodeBID)
{	
	//ScopedLock lock(mUEListLock);
	UEInfo *uep=nullptr;
	for (auto itr = mUEList.begin(); itr == mUEList.end() ? 0 : ((uep = *itr++), 1);){	
	//RN_FOR_ALL(UEList_t, mUEList, uep) {
		if (ueidtypeCRNTI) {
			if (uehandle == uep->mCRNTI && mNodeBID == uep->mNodeBID) { 
				return uep; 
				}
		}
		else {
			if (uehandle == uep->mURNTI && mNodeBID == uep->mNodeBID) { 
				return uep; 
				}
		}
	}
	return NULL;
}

// The crnti is 16 bits for UE id and the urnti is 12 bits for SRNC id and 20 bits for UE id.
// We will use the same UE id for both.
void Rrc::newRNTI(uint32_t *urnti, uint16_t *crnti) {
	unsigned ueid = ++mRrcRNTI; // skip 0.  We use 0 sometimes to mean undefined UNRTI.
	if (mRrcRNTI >= 65536) { mRrcRNTI = 0; }
	*crnti = ueid;
	unsigned srnc = UMTS_SRNC_ID;
	//printf("SRNC=%d ueid=%d\n",srnc,ueid);	// Something is wrong.
	*urnti = (srnc << 20) | ueid;
}

// Interpreting 25.331 10.3.3.15 InitialUEIdentity.
// This is used in the RRC Intial Connection Request, used when UE is in idle mode.
// If we get an identical AsnId, we want to use the identical URNTI by returning the existing UEInfo,
// in case the UE sends second request before we finish the first request we dont confuse it.
// Other than that, I think it is ok to issue a new URNTI every time we get the RRC Initial Connection Request.
// The UE may also be in idle mode because we goofed up and lost it, but in that case I dont
// think it matters if we issue a new URNTI or not.
//UEInfo *Rrc::findUeByAsnId(AsnUeId *asnId)
//{
//	{
//		ScopedLock lock(mUEListLock);
//		UEInfo *uep=nullptr;
//		RN_FOR_ALL(UEList_t, mUEList, uep) {
//			// If the whole thing matches just use it.
//			// The UE may identify itself one way (eg IMSI) on the first rrc connection request,
//			// then later use TMSI or P-TMSI.
//			// The UE may identify itself by P-TMSI using a P-TMSI that it obtained from us days ago.
//			// None of that matters; we are only trying to identify identical RRC Intial Connection Requests
//			// from the same UE.
//			if (asnId->eql(uep->mUid)) { return uep; }
//		}
//	}
//	//HKZ-NOT IMPL
//	return NULL;
//}

