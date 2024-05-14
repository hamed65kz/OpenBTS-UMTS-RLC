#include "URlc.h"
#include "RRC/URRC.h"
//#include <queue>
#include "RLC/Tran/URlcTransUm.h"
#include "MAC/MACEngine.h"
#include "Utilities/Logger.h"

std::queue<BitVector*> URlc::RxCCCHQueue ;


void URlc::initRLCConfigs()
{
	gLogInit("openbts-umts", "NOTICE", LOG_LOCAL7);
	rrcInitCommonCh(UMTS_PRACH_SF,UMTS_SCCPCH_SF);
	setupDlRlcCCCH();
}
void URlc::macPushUpRXDCCH(BitVector pdu, RbId rbid, int UEid){
	//URNTI = 0;
	//CRNTI = 1;
	int ueidtype = 0;
	UEInfo *uep = gRrc.findUe(ueidtype, UEid);
	if (uep == NULL) {
		LOG(INFO) << "Could not find UE with id " << UEid;
		cout << " - Could not find UE with id " << UEid;
		return;
	}
	else{
		int a = 0;
	}
	uep->ueWriteLowSide(rbid, pdu, stCELL_FACH);
}

void URlc::macPushUpRxCCCH(BitVector *pdu){
	RxCCCHQueue.push(pdu);
}


BitVector* URlc::rrcRecvCCCH()
{
	if (RxCCCHQueue.size() > 0)
	{
		BitVector* first = RxCCCHQueue.front();
		RxCCCHQueue.pop();
		return first;
	}
	return nullptr;
}
/**
Blocking read.
@return Pointer to object (will not be NULL).
*/
RrcUplinkMessage* URlc::rrcRecvDCCH(){
	RrcUplinkMessage *msg = gRrcUplinkQueue.read();
	//UEInfo* ue = msg->msgUep;
	return msg;
}
void URlc::rrcSendRRCConnectionSetup(int urnti,ByteVector sdu)
{
	//UEInfo *uep = gRrc.findUeByAsnId(&aid);
	UEInfo *uep = gRrc.findUe(false, urnti);
	if (uep == NULL) {
		uep = new UEInfo(urnti);
		//comment = "UL_CCCH_MessageType_PR_rrcConnectionRequest (new UE)";
	}
	else{
		int a = 0;
	}
	//uep->ueSetState(stIdleMode);
	uep->ueConnectRlc(gRrcDcchConfig, stCELL_FACH);	
	const std::string descrRrcConnectionSetup("RRC_Connection_Setup_Message");
	rrcSendCCCH(sdu, descrRrcConnectionSetup);
}
void URlc::rrcSendCCCH(ByteVector sdu, std::string desc)
{
	writeHighSideCcch(sdu, desc);
}
void URlc::rrcSendDCCH(ByteVector sdu, int UEid, RbId rbid, std::string desc)
{
	int ueidtype = 0;
	UEInfo *uep = gRrc.findUe(ueidtype, UEid);
	if (uep == NULL) {
		LOG(INFO) << "Could not find UE with id " << UEid;
		return;
	}
	uep->ueWriteHighSide((RbId)rbid, sdu, desc);
}
vector<ByteVector*> URlc::macReadTx(){
	vector<ByteVector*> dcch_pdus = flushUE();
	ByteVector* ccch_pdu = flushQ();
	if (ccch_pdu)
	{
		dcch_pdus.push_back(ccch_pdu);
	}
	return dcch_pdus;
}