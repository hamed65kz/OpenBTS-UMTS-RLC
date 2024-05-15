#include "MACEngine.h"
#include "../Ue/UEInfo.h"
#include "../RRC/URRC.h"

// Just look for a pdu, any pdu, and send it.
vector<ByteVector*> flushUE()
{
	UEInfo *uep =nullptr;
	//printf("BEFORE LOCK\n");
	ScopedLock lock(gRrc.mUEListLock);
	//printf("AFTER LOCK\n");
	//printf("LIST size=%d\n",gRrc.mUEList.size());
	vector<ByteVector*> pdus;
	RN_FOR_ALL(Rrc::UEList_t, gRrc.mUEList, uep) {
		//if (uep->ueGetState() != stCELL_FACH) { continue; }
		//if (gMacSwitch.pickFachMac(uep->mURNTI) != this) { continue; }//HKZ
		// Look in each logical channel.
		// The ones that might have something in them are 1,2,3.
		// Currently we dont hook up 5 and above in CELL_FACH state,
		// but we'll just check all anyway.  This may be wrong.
		RN_UE_FOR_ALL_RLC_DOWN(uep, rbid, rlcp) {
			ByteVector *pdu = rlcp->rlcReadLowSide();
			if (!pdu) { continue; }
			LOG(INFO) << "Found RLC pdu on rb: " << rbid << " pdu: " << *pdu;

			// Format up a TransportBlock and send it off.
			// For this case we send a reference instead of a pointer to allocated.
			//MaccTbDl tb(macGetDlTrBkSz(), pdu, uep, rbid);//HKZ
			//sendDownstreamTb(tb);//HKZ
			pdus.push_back(pdu);
			//delete pdu;
			//return pdus;
		}
	}
	return pdus;
}
ByteVector* flushQ()
{
	// Now we can treat the ccch rlc like any other.
	ByteVector *pdu = macReadFromCCCH();
	return pdu;
}